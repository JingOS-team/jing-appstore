/***************************************************************************
 *   Copyright © 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.1
import org.kde.discover 1.0
import org.kde.discover.app 1.0
import org.kde.kirigami 1.0 as Kirigami
import "navigation.js" as Navigation

Kirigami.GlobalDrawer {
    id: drawer
    anchors.fill: parent
    title: i18n("Discover")
    titleIcon: "plasmadiscover"
    bannerImageSource: "image://icon/plasma"

    function itemsFilter(actions, items) {
        var ret = [];
        for(var v in actions)
            ret.push(actions[v]);

        return ret.concat(items);
    }
    topContent: TextField {
        Layout.fillWidth: true

        enabled: window.stack.currentItem!=null && (window.stack.currentItem.searchFor!=null || window.stack.currentItem.hasOwnProperty("search"))
        focus: true

        placeholderText: (!enabled || window.stack.currentItem.title == "") ? i18n("Search...") : i18n("Search in '%1'...", window.stack.currentItem.title)
        onTextChanged: searchTimer.running = true

        Timer {
            id: searchTimer
            running: false
            repeat: false
            interval: 200
            onTriggered: {
                var curr = window.stack.currentItem;
                if (!curr.hasOwnProperty("search"))
                    Navigation.openApplicationList( { search: parent.text })
                else
                    curr.search = parent.text;
            }
        }
    }

    ColumnLayout {
        spacing: 0
        Layout.fillWidth: true

        Kirigami.BasicListItem {
            icon: installedAction.iconName
            label: installedAction.text
            onClicked: installedAction.trigger()
        }
        Kirigami.BasicListItem {
            icon: settingsAction.iconName
            label: settingsAction.text
            onClicked: settingsAction.trigger()
        }
        Kirigami.BasicListItem {
            icon: updateAction.iconName
            label: updateAction.text
            onClicked: updateAction.trigger()
        }
    }

    property var objects: []
    Instantiator {
        model: CategoryModel {}
        delegate: Kirigami.Action {
            text: display
            onTriggered: Navigation.openCategory(category)
        }

        onObjectAdded: {
            drawer.objects.push(object)
            drawer.actions = drawer.objects
        }
        onObjectRemoved: drawer.objects = drawer.objects.splice(drawer.objects.indexOf(object))
    }

    actions: objects
    modal: Helpers.isCompact
    handleVisible: Helpers.isCompact

    states: [
        State {
            name: "full"
            when: !Helpers.isCompact
            PropertyChanges { target: drawer; opened: true }
        }
    ]
}
