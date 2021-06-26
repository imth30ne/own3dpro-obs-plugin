// Integration of the OWN3D service into OBS Studio
// Copyright (C) 2020 own3d media GmbH <support@own3d.tv>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "ui.hpp"
#include <QMainWindow>
#include "plugin.hpp"

#include <obs-frontend-api.h>

static constexpr std::string_view I18N_THEMEBROWSER_MENU = "Menu.ThemeBrowser";

static constexpr std::string_view CFG_PRIVACYPOLICY = "privacypolicy";

inline void qt_init_resource()
{
	Q_INIT_RESOURCE(own3d);
}

inline void qt_cleanup_resource()
{
	Q_CLEANUP_RESOURCE(own3d);
}

own3d::ui::ui::~ui()
{
	obs_frontend_remove_event_callback(obs_event_handler, this);
	qt_cleanup_resource();
}

own3d::ui::ui::ui()
	: _gdpr(), _privacypolicy(false), _action(), _theme_browser(), _download(), _eventlist_dock(),
	  _eventlist_dock_action()
{
	qt_init_resource();
	obs_frontend_add_event_callback(obs_event_handler, this);

	{ // Load Configuration
		auto cfg = own3d::configuration::instance();

		// GDPR Flag
		obs_data_set_default_bool(cfg->get().get(), CFG_PRIVACYPOLICY.data(), false);
		_privacypolicy = obs_data_get_bool(cfg->get().get(), CFG_PRIVACYPOLICY.data());
	}
}

void own3d::ui::ui::obs_event_handler(obs_frontend_event event, void* private_data)
{
	own3d::ui::ui* ui = reinterpret_cast<own3d::ui::ui*>(private_data);
	if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
		ui->load();
	} else if (event == OBS_FRONTEND_EVENT_EXIT) {
		ui->unload();
	}
}

void own3d::ui::ui::load()
{
	{ // GDPR
		_gdpr = QSharedPointer<own3d::ui::gdpr>::create(reinterpret_cast<QMainWindow*>(obs_frontend_get_main_window()));
		_gdpr->connect(_gdpr.get(), &own3d::ui::gdpr::accepted, this, &own3d::ui::ui::on_gdpr_accept);
		_gdpr->connect(_gdpr.get(), &own3d::ui::gdpr::declined, this, &own3d::ui::ui::on_gdpr_decline);
	}

	{ // OWN3D Menu
		_action =
			reinterpret_cast<QAction*>(obs_frontend_add_tools_menu_qaction(D_TRANSLATE(I18N_THEMEBROWSER_MENU.data())));
		connect(_action, &QAction::triggered, this, &own3d::ui::ui::own3d_action_triggered);
	}

	{ // Create Theme Browser.
		_theme_browser = new own3d::ui::browser();
		connect(_theme_browser, &own3d::ui::browser::selected, this, &own3d::ui::ui::own3d_theme_selected);
	}

	{ // Event List Dock
		_eventlist_dock        = new own3d::ui::dock::eventlist();
		_eventlist_dock_action = _eventlist_dock->add_obs_dock();
	}

	// Verify that the user has accepted the privacy policy.
	if (!_privacypolicy) {
		_gdpr->show();
	}
}

void own3d::ui::ui::unload()
{
	{ // Event List Dock
		_eventlist_dock->deleteLater();
		_eventlist_dock        = nullptr;
		_eventlist_dock_action = nullptr;
	}

	{ // Theme Browser
		_theme_browser->deleteLater();
		disconnect(_theme_browser, &own3d::ui::browser::selected, this, &own3d::ui::ui::own3d_theme_selected);
		_theme_browser = nullptr;
	}

	{ // OWN3D Menu
		_action = nullptr;
	}

	if (_gdpr) { // GDPR
		_gdpr->deleteLater();
		_gdpr.reset();
	}
}

void own3d::ui::ui::on_gdpr_accept()
{
	_gdpr->close();
	_gdpr.reset();
	_privacypolicy = true;

	auto cfg = own3d::configuration::instance();
	obs_data_set_bool(cfg->get().get(), CFG_PRIVACYPOLICY.data(), _privacypolicy);
	cfg->save();
}

void own3d::ui::ui::on_gdpr_decline()
{
	_gdpr->close();
	_gdpr.reset();
	_privacypolicy = false;

	auto cfg = own3d::configuration::instance();
	obs_data_set_bool(cfg->get().get(), CFG_PRIVACYPOLICY.data(), _privacypolicy);
	cfg->save();

	static_cast<QMainWindow*>(obs_frontend_get_main_window())->close();
}

void own3d::ui::ui::own3d_action_triggered(bool)
{
	_theme_browser->show();
}

std::shared_ptr<own3d::ui::ui> own3d::ui::ui::_instance = nullptr;

void own3d::ui::ui::initialize()
{
	if (!own3d::ui::ui::_instance)
		own3d::ui::ui::_instance = std::make_shared<own3d::ui::ui>();
}

void own3d::ui::ui::finalize()
{
	own3d::ui::ui::_instance.reset();
}

std::shared_ptr<own3d::ui::ui> own3d::ui::ui::instance()
{
	return own3d::ui::ui::_instance;
}

void own3d::ui::ui::own3d_theme_selected(const QUrl& download_url, const QString& name, const QString& hash)
{ // We have a theme to download!
	// Hide the browser.
	_theme_browser->hide();

	// Recreate the download dialog.
	// FIXME! Don't recreate it, instead reset it.
	_download = new own3d::ui::installer(download_url, name, hash);
}
