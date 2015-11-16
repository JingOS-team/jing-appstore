/***************************************************************************
 *   Copyright © 2013 Lukas Appelhans <l.appelhans@gmx.de>                 *
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
#include "PackageKitUpdater.h"
#include "PackageKitMessages.h"

#include <PackageKit/Daemon>
#include <QDebug>
#include <QMessageBox>
#include <QAction>
#include <QSet>

#include <KLocalizedString>
#include <QIcon>

PackageKitUpdater::PackageKitUpdater(PackageKitBackend * parent)
  : AbstractBackendUpdater(parent),
    m_transaction(nullptr),
    m_backend(parent),
    m_isCancelable(false),
    m_isProgressing(false),
    m_speed(0),
    m_remainingTime(0),
    m_percentage(0),
    m_lastUpdate()
{
    m_updateAction = new QAction(this);
    m_updateAction->setIcon(QIcon::fromTheme(QStringLiteral("system-software-update")));
    m_updateAction->setText(i18nc("@action Checks the Internet for updates", "Check for Updates"));
    m_updateAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    m_updateAction->setEnabled(PackageKit::Daemon::networkState() != PackageKit::Daemon::NetworkOffline);
    connect(m_updateAction, &QAction::triggered, parent, &PackageKitBackend::refreshDatabase);

    fetchLastUpdateTime();
}

PackageKitUpdater::~PackageKitUpdater()
{
    delete m_transaction;
}

void PackageKitUpdater::prepare()
{
    Q_ASSERT(!m_transaction);
    m_toUpgrade = m_backend->upgradeablePackages().toSet();
    m_allUpgradeable = m_toUpgrade;
}

void PackageKitUpdater::setTransaction(PackageKit::Transaction* transaction)
{
    m_transaction = transaction;
    m_isCancelable = transaction->allowCancel();

    connect(m_transaction.data(), &PackageKit::Transaction::finished, this, &PackageKitUpdater::finished);
    connect(m_transaction.data(), &PackageKit::Transaction::errorCode, this, &PackageKitUpdater::errorFound);
    connect(m_transaction.data(), &PackageKit::Transaction::mediaChangeRequired, this, &PackageKitUpdater::mediaChange);
    connect(m_transaction.data(), &PackageKit::Transaction::requireRestart, this, &PackageKitUpdater::requireRestart);
    connect(m_transaction.data(), &PackageKit::Transaction::eulaRequired, this, &PackageKitUpdater::eulaRequired);
    connect(m_transaction.data(), &PackageKit::Transaction::statusChanged, this, &PackageKitUpdater::statusChanged);
    connect(m_transaction.data(), &PackageKit::Transaction::speedChanged, this, &PackageKitUpdater::speedChanged);
    connect(m_transaction.data(), &PackageKit::Transaction::allowCancelChanged, this, &PackageKitUpdater::cancellableChanged);
    connect(m_transaction.data(), &PackageKit::Transaction::remainingTimeChanged, this, &PackageKitUpdater::remainingTimeChanged);
    connect(m_transaction.data(), &PackageKit::Transaction::percentageChanged, this, &PackageKitUpdater::percentageChanged);
}

QSet<AbstractResource*> PackageKitUpdater::packagesForPackageId(const QSet<QString>& pkgids) const
{
    QSet<QString> packages;
    foreach(const QString& pkgid, pkgids) {
        packages += PackageKit::Daemon::packageName(pkgid);
    }

    QSet<AbstractResource*> ret;
    foreach (AbstractResource * res, m_allUpgradeable) {
        PackageKitResource* pres = qobject_cast<PackageKitResource*>(res);
        if (packages.contains(pres->allPackageNames().toSet())) {
            ret.insert(res);
        }
    }

    return ret;
}

QSet<QString> PackageKitUpdater::involvedPackages(const QSet<AbstractResource*>& packages) const
{
    QSet<QString> packageIds;
    packageIds.reserve(packages.size());
    foreach (AbstractResource * res, packages) {
        PackageKitResource * app = qobject_cast<PackageKitResource*>(res);
        QString pkgid = m_backend->upgradeablePackageId(app);
        packageIds.insert(pkgid);
    }
    return packageIds;
}

void PackageKitUpdater::start()
{
    setTransaction(PackageKit::Daemon::updatePackages(involvedPackages(m_toUpgrade).toList()));
    setProgressing(true);
}

void PackageKitUpdater::finished(PackageKit::Transaction::Exit exit, uint )
{
    if (exit == PackageKit::Transaction::ExitEulaRequired)
        return;
    disconnect(m_transaction, nullptr, this, nullptr);
    m_transaction = nullptr;

    setProgressing(false);
    m_backend->refreshDatabase();
    fetchLastUpdateTime();
}

void PackageKitUpdater::cancellableChanged()
{
    if (m_isCancelable != m_transaction->allowCancel()) {
        m_isCancelable = m_transaction->allowCancel();
        emit cancelableChanged(m_isCancelable);
    }
}

void PackageKitUpdater::percentageChanged()
{
    if (m_percentage != m_transaction->percentage()) {
        m_percentage = m_transaction->percentage();
        emit progressChanged(m_percentage);
    }
}

void PackageKitUpdater::remainingTimeChanged()
{
    if (m_remainingTime != m_transaction->remainingTime()) {
        m_remainingTime = m_transaction->remainingTime();
        emit remainingTimeChanged();
    }
}

void PackageKitUpdater::speedChanged()
{
    if (m_speed != m_transaction->speed()) {
        m_speed = m_transaction->speed();
        emit downloadSpeedChanged(m_speed);
    }
}

void PackageKitUpdater::statusChanged()
{
    if (m_status != m_transaction->status()) {
        m_status = m_transaction->status();
        m_statusMessage = PackageKitMessages::statusMessage(m_status);
        m_statusDetail = PackageKitMessages::statusDetail(m_status);

        emit statusMessageChanged(m_statusMessage);
        emit statusDetailChanged(m_statusDetail);
    }
}

bool PackageKitUpdater::hasUpdates() const
{
    return m_backend->updatesCount() > 0;
}

qreal PackageKitUpdater::progress() const
{
    return m_percentage;
}

/** proposed ETA in milliseconds */
long unsigned int PackageKitUpdater::remainingTime() const
{
    return m_remainingTime;
}

void PackageKitUpdater::removeResources(const QList<AbstractResource*>& apps)
{
    QSet<QString> pkgs = involvedPackages(apps.toSet());
    m_toUpgrade.subtract(packagesForPackageId(pkgs));
}

void PackageKitUpdater::addResources(const QList<AbstractResource*>& apps)
{
    QSet<QString> pkgs = involvedPackages(apps.toSet());
    m_toUpgrade.unite(packagesForPackageId(pkgs));
}

QList<AbstractResource*> PackageKitUpdater::toUpdate() const
{
    return m_toUpgrade.toList();
}

bool PackageKitUpdater::isMarked(AbstractResource* res) const
{
    return m_toUpgrade.contains(res);
}

QDateTime PackageKitUpdater::lastUpdate() const
{
    return m_lastUpdate;
}

bool PackageKitUpdater::isAllMarked() const
{
    return m_toUpgrade.count() >= m_backend->updatesCount();
}

bool PackageKitUpdater::isCancelable() const
{
    return m_transaction->allowCancel();
}

bool PackageKitUpdater::isProgressing() const
{
    return m_isProgressing;
}

QString PackageKitUpdater::statusMessage() const
{
    return m_statusMessage;
}

QString PackageKitUpdater::statusDetail() const
{
    return m_statusDetail;
}

quint64 PackageKitUpdater::downloadSpeed() const
{
    return m_speed;
}

QList<QAction*> PackageKitUpdater::messageActions() const
{
    return QList<QAction*>() << m_updateAction;
}

void PackageKitUpdater::cancel()
{
    m_transaction->cancel();
}

void PackageKitUpdater::errorFound(PackageKit::Transaction::Error err, const QString& error)
{
    Q_UNUSED(error);
    if (err == PackageKit::Transaction::ErrorNoLicenseAgreement)
        return;
    QMessageBox::critical(nullptr, i18n("PackageKit error found"), PackageKitMessages::errorMessage(err));
    qWarning() << "Error happened" << err << error;
}

void PackageKitUpdater::mediaChange(PackageKit::Transaction::MediaType media, const QString& type, const QString& text)
{
    Q_UNUSED(media)
    QMessageBox::information(nullptr, i18n("PackageKit media change"), i18n("Media Change of type '%1' is requested.\n%2", type, text));
}

void PackageKitUpdater::requireRestart(PackageKit::Transaction::Restart restart, const QString& pkgid)
{
    QMessageBox::information(nullptr, i18n("PackageKit restart required"), PackageKitMessages::restartMessage(restart, pkgid));
}

void PackageKitUpdater::eulaRequired(const QString& eulaID, const QString& packageID, const QString& vendor, const QString& licenseAgreement)
{
    QString packageName = PackageKit::Daemon::packageName(packageID);
    int ret = QMessageBox::question(nullptr, i18n("%1 requires user to accept its license", packageName), i18n("The package %1 and its vendor %2 require that you accept their license:\n %3",
                                                 packageName, vendor, licenseAgreement), QMessageBox::Yes, QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        PackageKit::Transaction* t = PackageKit::Daemon::acceptEula(eulaID);
        connect(t, &PackageKit::Transaction::finished, this, &PackageKitUpdater::start);
    } else {
        finished(PackageKit::Transaction::ExitCancelled, 0);
    }
}

void PackageKitUpdater::setProgressing(bool progressing)
{
    if (m_isProgressing != progressing) {
        m_isProgressing = progressing;
        emit progressingChanged(m_isProgressing);
    }
}

void PackageKitUpdater::fetchLastUpdateTime()
{
    QDBusPendingReply<uint> transaction = PackageKit::Daemon::global()->getTimeSinceAction(PackageKit::Transaction::RoleGetUpdates);
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(transaction, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &PackageKitUpdater::lastUpdateTimeReceived);
}

void PackageKitUpdater::lastUpdateTimeReceived(QDBusPendingCallWatcher* w)
{
    QDBusPendingReply<uint> reply = w->reply();
    if (reply.isError()) {
        qWarning() << "Error when fetching the last update time" << reply.error();
    } else {
        m_lastUpdate = QDateTime::currentDateTime().addSecs(-reply.value());
    }
    w->deleteLater();
}