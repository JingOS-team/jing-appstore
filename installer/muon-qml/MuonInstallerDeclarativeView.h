/*
 *   Copyright (C) 2012 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef MUONINSTALLERDECLARATIVEVIEW_H
#define MUONINSTALLERDECLARATIVEVIEW_H

#include <QtDeclarative/QDeclarativeView>

class MuonInstallerDeclarativeView : public QDeclarativeView
{
    Q_OBJECT
    public:
        explicit MuonInstallerDeclarativeView(QWidget* parent = 0);
};

#endif // MUONINSTALLERDECLARATIVEVIEW_H
