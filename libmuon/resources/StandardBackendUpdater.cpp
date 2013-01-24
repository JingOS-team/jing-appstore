/*
 *   Copyright (C) 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
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

#include <resources/StandardBackendUpdater.h>
#include <resources/AbstractResourcesBackend.h>
#include <resources/AbstractResource.h>
#include <QDateTime>

StandardBackendUpdater::StandardBackendUpdater(AbstractResourcesBackend* parent)
    : AbstractBackendUpdater(parent)
    , m_backend(parent)
{}

bool StandardBackendUpdater::hasUpdates() const
{
    return m_backend->updatesCount() > 0;
}

void StandardBackendUpdater::start()
{
    foreach(AbstractResource* res, m_toUpgrade) {
        m_backend->installApplication(res);
    }
    //TODO: do that when all resources have been notified as installed by the backend
    emit updatesFinnished();
}

qreal StandardBackendUpdater::progress() const
{
    return hasUpdates() ? 0 : 100;
}

long unsigned int StandardBackendUpdater::remainingTime() const
{
    return 0;
}

void StandardBackendUpdater::prepare()
{
    m_toUpgrade = m_backend->upgradeablePackages().toSet();
}

void StandardBackendUpdater::addResources(const QList< AbstractResource* >& apps)
{
    m_toUpgrade += apps.toSet();
}

void StandardBackendUpdater::removeResources(const QList< AbstractResource* >& apps)
{
    m_toUpgrade -= apps.toSet();
}

void StandardBackendUpdater::cleanup()
{
    m_toUpgrade.clear();
}

QList<AbstractResource*> StandardBackendUpdater::toUpdate() const
{
    return m_toUpgrade.toList();
}

bool StandardBackendUpdater::isAllMarked() const
{
    //Maybe we should make this smarter...
    return m_preparedSize<=m_toUpgrade.size();
}

QDateTime StandardBackendUpdater::lastUpdate() const
{
    return QDateTime();
}

bool StandardBackendUpdater::isCancelable() const
{
    //We don't really know when we can cancel, so we never let
    return false;
}

bool StandardBackendUpdater::isProgressing() const
{
    return false;
}

QString StandardBackendUpdater::statusDetail() const
{
    return QString();
}

QString StandardBackendUpdater::statusMessage() const
{
    return QString();
}

