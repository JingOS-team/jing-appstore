/***************************************************************************
 *   Copyright © 2010 Jonathan Thomas <echidnaman@kubuntu.org>             *
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

#include "ApplicationExtender.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>

#include <KDebug>
#include <KIcon>
#include <KLocale>

#include <LibQApt/Backend>

#include "Application.h"

ApplicationExtender::ApplicationExtender(QWidget *parent, Application *app, QApt::Backend *backend)
    : QWidget(parent)
    , m_app(app)
    , m_backend(backend)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    setLayout(layout);

    QPushButton *infoButton = new QPushButton(this);
    infoButton->setText(i18n("More Info"));
    connect(infoButton, SIGNAL(clicked()), this, SLOT(emitInfoButtonClicked()));

    QWidget *buttonSpacer = new QWidget(this);
    buttonSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    m_actionButton = new QPushButton(this);

    if (app->package()->isInstalled()) {
        m_actionButton->setIcon(KIcon("edit-delete"));
        m_actionButton->setText(i18n("Remove"));
        connect(m_actionButton, SIGNAL(clicked()), this, SLOT(emitRemoveButtonClicked()));
    } else {
        m_actionButton->setIcon(KIcon("download"));
        m_actionButton->setText(i18n("Install"));
        connect(m_actionButton, SIGNAL(clicked()), this, SLOT(emitInstallButtonClicked()));
    }

    m_progressBar = new QProgressBar(this);
    m_progressBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    m_progressBar->hide();

    layout->addWidget(infoButton);
    layout->addWidget(buttonSpacer);
    layout->addWidget(m_actionButton);
    layout->addWidget(m_progressBar);

    connect(m_backend, SIGNAL(workerEvent(QApt::WorkerEvent)),
            this, SLOT(workerEvent(QApt::WorkerEvent)));
}

ApplicationExtender::~ApplicationExtender()
{
}

void ApplicationExtender::workerEvent(QApt::WorkerEvent event)
{
    switch (event) {
    case QApt::PackageDownloadStarted:
        m_actionButton->hide();
        m_progressBar->show();
        m_progressBar->setFormat(i18nc("@info:status", "Downloading"));
        connect(m_backend, SIGNAL(downloadProgress(int, int, int)),
                this, SLOT(updateProgress(int)));
        break;
    case QApt::PackageDownloadFinished:
        disconnect(m_backend, SIGNAL(downloadProgress(int, int, int)),
                   this, SLOT(updateProgress(int)));
        break;
    case QApt::CommitChangesStarted:
        m_progressBar->setValue(0);
        if (!m_app->package()->isInstalled()) {
            m_progressBar->setFormat(i18nc("@info:status", "Installing"));
        } else {
            m_progressBar->setFormat(i18nc("@info:status", "Removing"));
        }
        connect(m_backend, SIGNAL(commitProgress(const QString &, int)),
                this, SLOT(updateProgress(const QString &, int)));
        break;
    case QApt::CommitChangesFinished:
        disconnect(m_backend, SIGNAL(commitProgress(const QString &, int)),
                   this, SLOT(updateProgress(const QString &, int)));
        break;
    default:
        break;
    }
}

void ApplicationExtender::updateProgress(int percentage)
{
    m_progressBar->setValue(percentage);
}

void ApplicationExtender::updateProgress(const QString &text, int percentage)
{
    Q_UNUSED(text);
    updateProgress(percentage);
}

void ApplicationExtender::emitInfoButtonClicked()
{
    emit infoButtonClicked(m_app);
}

void ApplicationExtender::emitRemoveButtonClicked()
{
    emit removeButtonClicked(m_app);
}

void ApplicationExtender::emitInstallButtonClicked()
{
    emit installButtonClicked(m_app);
}

#include "ApplicationExtender.moc"
