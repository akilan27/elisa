/*
 * Copyright 2015 Matthieu Gallien <matthieu_gallien@yahoo.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "mediaplaylist.h"
#include "databaseinterface.h"
#include "musicaudiotrack.h"

#include <QtCore/QUrl>
#include <QtCore/QPersistentModelIndex>
#include <QtCore/QList>
#include <QtCore/QDebug>

#include <algorithm>

class MediaPlayListPrivate
{
public:

    QList<MediaPlayListEntry> mData;

    QList<MusicAudioTrack> mTrackData;

    DatabaseInterface *mMusicDatabase = nullptr;

    bool mUseLocalIcons = false;

};

MediaPlayList::MediaPlayList(QObject *parent) : QAbstractListModel(parent), d(new MediaPlayListPrivate)
{
}

MediaPlayList::~MediaPlayList()
{
    delete d;
}

int MediaPlayList::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->mData.size();
}

QVariant MediaPlayList::data(const QModelIndex &index, int role) const
{
    auto result = QVariant();

    if (!d->mMusicDatabase) {
        return result;
    }

    if (!index.isValid()) {
        return result;
    }

    if (index.row() < 0 || index.row() >= d->mData.size()) {
        return result;
    }

    if (role < ColumnsRoles::IsValidRole || role > ColumnsRoles::HasAlbumHeader) {
        return result;
    }

    ColumnsRoles convertedRole = static_cast<ColumnsRoles>(role);

    if (d->mData[index.row()].mIsValid) {
        switch(convertedRole)
        {
        case ColumnsRoles::IsValidRole:
            result = d->mData[index.row()].mIsValid;
            break;
        case ColumnsRoles::TitleRole:
            result = d->mTrackData[index.row()].title();
            break;
        case ColumnsRoles::DurationRole:
        {
            QTime trackDuration = d->mTrackData[index.row()].duration();
            if (trackDuration.hour() == 0) {
                result = trackDuration.toString(QStringLiteral("mm:ss"));
            } else {
                result = trackDuration.toString();
            }
            break;
        }
        case ColumnsRoles::MilliSecondsDurationRole:
            result = d->mTrackData[index.row()].duration().msecsSinceStartOfDay();
            break;
        case ColumnsRoles::ArtistRole:
            result = d->mTrackData[index.row()].artist();
            break;
        case ColumnsRoles::AlbumRole:
            result = d->mTrackData[index.row()].albumName();
            break;
        case ColumnsRoles::TrackNumberRole:
            result = d->mTrackData[index.row()].trackNumber();
            break;
        case ColumnsRoles::DiscNumberRole:
            if (d->mTrackData[index.row()].discNumber() > 0) {
                result = d->mTrackData[index.row()].discNumber();
            }
            break;
        case ColumnsRoles::ResourceRole:
            result = d->mTrackData[index.row()].resourceURI();
            break;
        case ColumnsRoles::ImageRole:
        {
            auto albumArt = d->mTrackData[index.row()].albumCover();
            if (albumArt.isValid()) {
                result = albumArt;
            } else {
                if (d->mUseLocalIcons) {
                    result = QUrl(QStringLiteral("qrc:/media-optical-audio.svg"));
                } else {
                    result = QUrl(QStringLiteral("image://icon/media-optical-audio"));
                }
            }
            break;
        }
        case ColumnsRoles::HasAlbumHeader:
            result = rowHasHeader(index.row());
            break;
        case ColumnsRoles::RatingRole:
            break;
        case ColumnsRoles::CountRole:
            break;
        case ColumnsRoles::CreatorRole:
            break;
        case ColumnsRoles::IsPlayingRole:
            result = d->mData[index.row()].mIsPlaying;
            break;
        }
    } else {
        switch(convertedRole)
        {
        case ColumnsRoles::IsValidRole:
            result = d->mData[index.row()].mIsValid;
            break;
        case ColumnsRoles::TitleRole:
            result = d->mData[index.row()].mTitle;
            break;
        case ColumnsRoles::IsPlayingRole:
            result = d->mData[index.row()].mIsPlaying;
            break;
        case ColumnsRoles::ArtistRole:
            result = d->mData[index.row()].mArtist;
            break;
        case ColumnsRoles::AlbumRole:
            result = d->mData[index.row()].mAlbum;
            break;
        case ColumnsRoles::TrackNumberRole:
            result = -1;
            break;
        case ColumnsRoles::HasAlbumHeader:
            result = rowHasHeader(index.row());
            break;
        case ColumnsRoles::DurationRole:
            break;
        case ColumnsRoles::DiscNumberRole:
            break;
        case ColumnsRoles::MilliSecondsDurationRole:
            break;
        case ColumnsRoles::ResourceRole:
            break;
        case ColumnsRoles::RatingRole:
            break;
        case ColumnsRoles::CountRole:
            break;
        case ColumnsRoles::CreatorRole:
            break;
        case ColumnsRoles::ImageRole:
            result = QStringLiteral("");
            break;
        }
    }

    return result;
}

bool MediaPlayList::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool modelModified = false;

    if (!index.isValid()) {
        return modelModified;
    }

    if (index.row() < 0 || index.row() >= d->mData.size()) {
        return modelModified;
    }

    if (role < ColumnsRoles::IsValidRole || role > ColumnsRoles::HasAlbumHeader) {
        return modelModified;
    }

    ColumnsRoles convertedRole = static_cast<ColumnsRoles>(role);

    switch(convertedRole)
    {
    case ColumnsRoles::IsPlayingRole:
        modelModified = true;
        d->mData[index.row()].mIsPlaying = value.toBool();
        Q_EMIT dataChanged(index, index, {role});
        break;
    default:
        modelModified = false;
    }

    return modelModified;
}

QHash<int, QByteArray> MediaPlayList::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[static_cast<int>(ColumnsRoles::IsValidRole)] = "isValid";
    roles[static_cast<int>(ColumnsRoles::TitleRole)] = "title";
    roles[static_cast<int>(ColumnsRoles::DurationRole)] = "duration";
    roles[static_cast<int>(ColumnsRoles::ArtistRole)] = "artist";
    roles[static_cast<int>(ColumnsRoles::AlbumRole)] = "album";
    roles[static_cast<int>(ColumnsRoles::TrackNumberRole)] = "trackNumber";
    roles[static_cast<int>(ColumnsRoles::DiscNumberRole)] = "discNumber";
    roles[static_cast<int>(ColumnsRoles::RatingRole)] = "rating";
    roles[static_cast<int>(ColumnsRoles::ImageRole)] = "image";
    roles[static_cast<int>(ColumnsRoles::CountRole)] = "count";
    roles[static_cast<int>(ColumnsRoles::IsPlayingRole)] = "isPlaying";
    roles[static_cast<int>(ColumnsRoles::HasAlbumHeader)] = "hasAlbumHeader";

    return roles;
}

bool MediaPlayList::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);

    bool hadAlbumHeader = false;

    if (rowCount() > row + count) {
        hadAlbumHeader = rowHasHeader(row + count);
    }

    for (int i = row, cpt = 0; cpt < count; ++i, ++cpt) {
        d->mData.removeAt(i);
        d->mTrackData.removeAt(i);
    }
    endRemoveRows();

    if (hadAlbumHeader != rowHasHeader(row)) {
        qDebug() << "dataChanged(index(row, 0), index(row, 0), {ColumnsRoles::HasAlbumHeader});" << row;
        Q_EMIT dataChanged(index(row, 0), index(row, 0), {ColumnsRoles::HasAlbumHeader});
    }

    Q_EMIT persistentStateChanged();

    return false;
}

void MediaPlayList::enqueue(qulonglong newTrackId)
{
    enqueue(MediaPlayListEntry(newTrackId));
}

void MediaPlayList::clearAndEnqueue(qulonglong newTrackId)
{
    clearPlayList();
    enqueue(MediaPlayListEntry(newTrackId));
}

void MediaPlayList::enqueue(MediaPlayListEntry newEntry)
{
    if (!d->mMusicDatabase) {
        return;
    }

    beginInsertRows(QModelIndex(), d->mData.size(), d->mData.size());
    d->mData.push_back(newEntry);
    d->mTrackData.push_back({});
    endInsertRows();

    emit persistentStateChanged();

    if (d->mMusicDatabase && !newEntry.mIsValid) {
        auto newTrackId = d->mMusicDatabase->trackIdFromTitleAlbumArtist(newEntry.mTitle, newEntry.mAlbum, newEntry.mArtist);

        if (newTrackId != 0) {
            d->mData.last().mId = newTrackId;
            d->mTrackData.last() = d->mMusicDatabase->trackFromDatabaseId(newTrackId);
            d->mData.last().mIsValid = true;

            Q_EMIT dataChanged(index(rowCount() - 1, 0), index(rowCount() - 1, 0), {});
        }
    } else {
        if (d->mMusicDatabase && newEntry.mIsValid) {
            d->mTrackData.last() = d->mMusicDatabase->trackFromDatabaseId(newEntry.mId);

            Q_EMIT dataChanged(index(rowCount() - 1, 0), index(rowCount() - 1, 0), {});
        }
    }

    Q_EMIT trackHasBeenAdded(data(index(d->mData.size() - 1, 0), ColumnsRoles::TitleRole).toString(), data(index(d->mData.size() - 1, 0), ColumnsRoles::ImageRole).toUrl());

    if (!newEntry.mIsValid) {
        Q_EMIT dataChanged(index(rowCount() - 1, 0), index(rowCount() - 1, 0), {MediaPlayList::HasAlbumHeader});
    }
}

bool MediaPlayList::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (sourceParent != destinationParent) {
        return false;
    }

    if (!beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild)) {
        return false;
    }

    auto firstMovedTrackHasHeader = rowHasHeader(sourceRow);
    auto nextTrackHasHeader = rowHasHeader(sourceRow + count);
    auto futureNextTrackHasHeader = rowHasHeader(destinationChild);
    if (sourceRow < destinationChild) {
        nextTrackHasHeader = rowHasHeader(sourceRow + count);
    }

    for (auto cptItem = 0; cptItem < count; ++cptItem) {
        if (sourceRow < destinationChild) {
            d->mData.move(sourceRow, destinationChild - 1);
            d->mTrackData.move(sourceRow, destinationChild - 1);
        } else {
            d->mData.move(sourceRow, destinationChild);
            d->mTrackData.move(sourceRow, destinationChild);
        }
    }

    endMoveRows();

    if (sourceRow < destinationChild) {
        if (firstMovedTrackHasHeader != rowHasHeader(destinationChild - count)) {
            Q_EMIT dataChanged(index(destinationChild - count, 0), index(destinationChild - count, 0), {ColumnsRoles::HasAlbumHeader});
        }
    } else {
        if (firstMovedTrackHasHeader != rowHasHeader(destinationChild)) {
            Q_EMIT dataChanged(index(destinationChild, 0), index(destinationChild, 0), {ColumnsRoles::HasAlbumHeader});
        }
    }

    if (sourceRow < destinationChild) {
        if (nextTrackHasHeader != rowHasHeader(sourceRow)) {
            Q_EMIT dataChanged(index(sourceRow, 0), index(sourceRow, 0), {ColumnsRoles::HasAlbumHeader});
        }
    } else {
        if (nextTrackHasHeader != rowHasHeader(sourceRow + count)) {
            Q_EMIT dataChanged(index(sourceRow + count, 0), index(sourceRow + count, 0), {ColumnsRoles::HasAlbumHeader});
        }
    }

    if (sourceRow < destinationChild) {
        if (futureNextTrackHasHeader != rowHasHeader(destinationChild + count - 1)) {
            Q_EMIT dataChanged(index(destinationChild + count - 1, 0), index(destinationChild + count - 1, 0), {ColumnsRoles::HasAlbumHeader});
        }
    } else {
        if (futureNextTrackHasHeader != rowHasHeader(destinationChild + count)) {
            Q_EMIT dataChanged(index(destinationChild + count, 0), index(destinationChild + count, 0), {ColumnsRoles::HasAlbumHeader});
        }
    }

    Q_EMIT persistentStateChanged();

    return true;
}

void MediaPlayList::move(int from, int to, int n)
{
    if (from < to) {
        moveRows({}, from, n, {}, to + 1);
    } else {
        moveRows({}, from, n, {}, to);
    }
}

void MediaPlayList::enqueue(QString albumName, QString artistName)
{
    if (!d->mMusicDatabase) {
        return;
    }

    const auto currentAlbum = d->mMusicDatabase->albumFromTitleAndAuthor(albumName, artistName);

    const auto allTracksKeys = currentAlbum.tracksKeys();

    for (const auto oneTrackId : allTracksKeys) {
        enqueue(oneTrackId);
    }
}

void MediaPlayList::enqueue(QString artistName)
{
    if (!d->mMusicDatabase) {
        return;
    }

    qDebug() << "MediaPlayList::enqueue" << artistName;

    const auto allTracks = d->mMusicDatabase->tracksFromAuthor(artistName);

    qDebug() << "MediaPlayList::enqueue" << artistName << allTracks.size();

    for (const auto &oneTrack : allTracks) {
        qDebug() << "MediaPlayList::enqueue" << artistName << oneTrack.title() << oneTrack.albumName() << oneTrack.artist();
        enqueue({oneTrack.title(), oneTrack.albumName(), oneTrack.artist()});
    }
}

void MediaPlayList::clearAndEnqueue(QString albumName, QString artistName)
{
    if (!d->mMusicDatabase) {
        return;
    }

    clearPlayList();
    enqueue(albumName, artistName);
}

void MediaPlayList::clearAndEnqueue(QString artistName)
{
    if (!d->mMusicDatabase) {
        return;
    }

    clearPlayList();
    enqueue(artistName);
}

void MediaPlayList::clearPlayList()
{
    beginRemoveRows({}, 0, d->mData.count());
    d->mData.clear();
    d->mTrackData.clear();
    endRemoveRows();
}

DatabaseInterface *MediaPlayList::databaseInterface() const
{
    return d->mMusicDatabase;
}

QList<QVariant> MediaPlayList::persistentState() const
{
    auto result = QList<QVariant>();

    for (auto &oneEntry : d->mData) {
        auto oneData = QList<QString>();
        auto oneTrack = d->mMusicDatabase->trackFromDatabaseId(oneEntry.mId);

        oneData.push_back(oneTrack.title());
        oneData.push_back(oneTrack.albumName());
        oneData.push_back(oneTrack.artist());

        result.push_back(QVariant(oneData));
    }

    return result;
}

void MediaPlayList::setDatabaseInterface(DatabaseInterface *musicDatabase)
{
    if (d->mMusicDatabase == musicDatabase) {
        return;
    }

    if (d->mMusicDatabase) {
        disconnect(d->mMusicDatabase, &DatabaseInterface::endTrackAdded,
                   this, &MediaPlayList::endTrackAdded);
    }

    d->mMusicDatabase = musicDatabase;

    connect(d->mMusicDatabase, &DatabaseInterface::endTrackAdded,
            this, &MediaPlayList::endTrackAdded);

    emit databaseInterfaceChanged();
}

void MediaPlayList::setPersistentState(QList<QVariant> persistentState)
{
    qDebug() << "MediaPlayList::setPersistentState" << persistentState;

    for (auto &oneData : persistentState) {
        auto trackData = oneData.toStringList();
        if (trackData.size() != 3) {
            continue;
        }

        auto restoredTitle = trackData[0];
        auto restoredAlbum = trackData[1];
        auto restoredArtist = trackData[2];

        enqueue({restoredTitle, restoredAlbum, restoredArtist});
    }

    emit persistentStateChanged();
}

void MediaPlayList::removeSelection(QList<int> selection)
{
    std::sort(selection.begin(), selection.end());
    std::reverse(selection.begin(), selection.end());
    for (auto oneItem : selection) {
        removeRow(oneItem);
    }
}

void MediaPlayList::endTrackAdded(QVector<qulonglong> newTracks)
{
    Q_UNUSED(newTracks);

    for (int i = 0; i < d->mData.size(); ++i) {
        auto &oneEntry = d->mData[i];

        if (oneEntry.mIsValid) {
            continue;
        }

        auto restoredTitle = oneEntry.mTitle;
        auto restoredAlbum = oneEntry.mAlbum;
        auto restoredArtist = oneEntry.mArtist;

        auto newTrackId = d->mMusicDatabase->trackIdFromTitleAlbumArtist(restoredTitle, restoredAlbum, restoredArtist);

        if (newTrackId != 0) {
            oneEntry.mId = newTrackId;
            d->mTrackData[i] = d->mMusicDatabase->trackFromDatabaseId(newTrackId);
            oneEntry.mIsValid = true;

            Q_EMIT dataChanged(index(i, 0), index(i, 0), {});
        }
    }
}

bool MediaPlayList::rowHasHeader(int row) const
{
    if (row >= rowCount()) {
        return false;
    }

    if (row < 0) {
        return false;
    }

    if (row - 1 < 0) {
        return true;
    }

    auto currentAlbum = QString();
    if (d->mData[row].mIsValid) {
        qDebug() << d->mData[row].mId;
        qDebug() << d->mTrackData[row].albumName();
        currentAlbum = d->mTrackData[row].albumName();
    } else {
        currentAlbum = d->mData[row].mAlbum;
    }

    auto previousAlbum = QString();
    if (d->mData[row - 1].mIsValid) {
        previousAlbum = d->mTrackData[row - 1].albumName();
    } else {
        previousAlbum = d->mData[row - 1].mAlbum;
    }

    if (currentAlbum == previousAlbum) {
        return false;
    }

    return true;
}


#include "moc_mediaplaylist.cpp"
