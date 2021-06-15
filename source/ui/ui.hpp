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

#pragma once
#include <QAction>
#include <QSharedPointer>
#include <memory>
#include <obs-frontend-api.h>
#include "ui-browser.hpp"
#include "ui-dock-chat.hpp"
#include "ui-dock-eventlist.hpp"
#include "ui-download.hpp"
#include "ui-gdpr.hpp"

namespace own3d::ui {
	class ui : public QObject {
		Q_OBJECT;

		private:
		QSharedPointer<own3d::ui::gdpr> _gdpr;
		bool                            _gdpr_accepted;

		QAction*            _action;
		own3d::ui::browser* _theme_browser;

		own3d::ui::installer* _download;

		QSharedPointer<dock::eventlist> _eventlist_dock;
		QAction*                        _eventlist_dock_action;
		QSharedPointer<dock::chat>      _chat_dock;
		QAction*                        _chat_dock_action;

		public:
		~ui();
		ui();

		private:
		static void obs_event_handler(obs_frontend_event event, void* private_data);

		void load();

		void unload();

		private slots:
		; // Needed by some linters.
		void on_gdpr_accept();

		void on_gdpr_decline();

		void own3d_action_triggered(bool);

		void own3d_theme_selected(const QUrl& download_url, const QString& name, const QString& hash);

		private /* Singleton */:
		static std::shared_ptr<own3d::ui::ui> _instance;

		public /* Singleton */:
		static void initialize();

		static void finalize();

		static std::shared_ptr<own3d::ui::ui> instance();
	};
} // namespace own3d::ui
