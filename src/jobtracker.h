/*
 This file is part of Akonadi.

 Copyright (c) 2009 KDAB
 Author: Till Adam <adam@kde.org>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 USA.
 */

#ifndef AKONADICONSOLE_JOBTRACKER_H
#define AKONADICONSOLE_JOBTRACKER_H

#include <QObject>
#include <QDateTime>
#include <QList>
#include <QPair>
#include "libakonadiconsole_export.h"
class JobInfo
{
public:
    JobInfo() : parent(-1), state(Initial)
    {}
    bool operator==(const JobInfo &other) const
    {
        return name == other.name
               && parent == other.parent
               && type == other.type
               && timestamp == other.timestamp
               && state == other.state;
    }

    QString name;
    int parent;
    QString type;
    QDateTime timestamp;
    QDateTime startedTimestamp;
    QDateTime endedTimestamp;
    enum JobState {
        Initial = 0,
        Running,
        Ended,
        Failed
    };
    JobState state;
    QString error;
    QString debugString;
    QString stateAsString() const;
};

class LIBAKONADICONSOLE_EXPORT JobTracker : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Akonadi.JobTracker")

public:
    explicit JobTracker(const char *name, QObject *parent = nullptr);
    ~JobTracker();

    QStringList sessions() const;

    int idForSession(const QString &session) const;
    QString sessionForId(int id) const;

    JobInfo info(int id) const;

    /**
     * Returns the number of (sub)jobs of a session or another job.
     * (i.e. going down)
     */
    int jobCount(int parentId) const;
    int jobIdAt(int childPos, int parentId) const;

    /**
     * Returns the parent (job/session) ID for the given (job/session) ID,
     * or -1 if the ID has no parent.
     * (i.e. going up)
     */
    int parentId(int id) const;

    int rowForJob(int id, int parentId) const;

    bool isEnabled() const;

    void clear();

Q_SIGNALS:
    /** Emitted when a job (or session) is about to be added to the tracker.
      * @param pos the position of the job or session relative to the parent
      * @param parentId the id of that parent.
      * This makes it easy for the model to find and update the right
      * part of the model, for efficiency.
      */
    void aboutToAdd(int pos, int parentId);
    void added();

    /** Emitted when jobs (or sessiona) have been updated in the tracker.
    * The format is a list of pairs consisting of the position of the
    * job or session relative to the parent and the id of that parent.
    * This makes it easy for the model to find and update the right
    * part of the model, for efficiency.
    */
    void updated(const QList< QPair<int, int> > &updates);

public Q_SLOTS:
    Q_SCRIPTABLE void jobCreated(const QString &session, const QString &jobName, const QString &parentJob, const QString &jobType, const QString &debugString);
    Q_SCRIPTABLE void jobStarted(const QString &jobName);
    Q_SCRIPTABLE void jobEnded(const QString &jobName, const QString &error);
    Q_SCRIPTABLE void setEnabled(bool on);
    void signalUpdates(); // public for the unittest

private:
    QVector<int> childJobs(int parentId) const;
    int idForJob(const QString &job) const;

    class Private;
    Private *const d;
};

#endif /* JOBTRACKER_H_ */
