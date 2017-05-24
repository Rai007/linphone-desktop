/*
 * ConferenceModel.cpp
 * Copyright (C) 2017  Belledonne Communications, Grenoble, France
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  Created on: May 23, 2017
 *      Author: Ronan Abhamon
 */

#include <QDateTime>

#include "../../Utils.hpp"
#include "../core/CoreManager.hpp"
#include "ConferenceHelperModel.hpp"

#include "ConferenceModel.hpp"

using namespace std;

// =============================================================================

ConferenceModel::ConferenceModel (QObject *parent) : QAbstractListModel(parent) {}

int ConferenceModel::rowCount (const QModelIndex &index) const {
  return mSipAddresses.count();
}

QHash<int, QByteArray> ConferenceModel::roleNames () const {
  QHash<int, QByteArray> roles;
  roles[Qt::DisplayRole] = "$sipAddress";
  return roles;
}

QVariant ConferenceModel::data (const QModelIndex &index, int role) const {
  int row = index.row();

  if (!index.isValid() || row < 0 || row >= mSipAddresses.count())
    return QVariant();

  if (role == Qt::DisplayRole)
    return mSipAddresses[row];

  return QVariant();
}

// -----------------------------------------------------------------------------

void ConferenceModel::terminate () {
  shared_ptr<linphone::Core> core = CoreManager::getInstance()->getCore();
  core->terminateConference();

  for (const auto &call : core->getCalls()) {
    if (call->getParams()->getLocalConferenceMode())
      call->terminate();
  }
}

// -----------------------------------------------------------------------------

void ConferenceModel::startRecording () {
  if (mRecording)
    return;

  qInfo() << QStringLiteral("Start recording conference:") << this;

  CoreManager *coreManager = CoreManager::getInstance();
  coreManager->getCore()->startConferenceRecording(
    ::Utils::qStringToLinphoneString(
      QStringLiteral("%1%2.mkv")
      .arg(coreManager->getSettingsModel()->getSavedVideosFolder())
      .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss"))
    )
  );
  mRecording = true;

  emit recordingChanged(true);
}

void ConferenceModel::stopRecording () {
  if (!mRecording)
    return;

  qInfo() << QStringLiteral("Stop recording conference:") << this;

  mRecording = false;
  CoreManager::getInstance()->getCore()->stopConferenceRecording();

  emit recordingChanged(false);
}

// -----------------------------------------------------------------------------

bool ConferenceModel::getMicroMuted () const {
  return !CoreManager::getInstance()->getCore()->micEnabled();
}

void ConferenceModel::setMicroMuted (bool status) {
  shared_ptr<linphone::Core> core = CoreManager::getInstance()->getCore();

  if (status == core->micEnabled()) {
    core->enableMic(!status);
    emit microMutedChanged(status);
  }
}

// -----------------------------------------------------------------------------

bool ConferenceModel::getRecording () const {
  return mRecording;
}
