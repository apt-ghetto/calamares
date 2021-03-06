/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2020 Adriaan de Groot <groot@kde.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#include "Config.h"
#include "CreateUserJob.h"
#include "MiscJobs.h"

#include "GlobalStorage.h"
#include "JobQueue.h"
#include "utils/Logger.h"
#include "utils/Yaml.h"

#include <QDir>
#include <QtTest/QtTest>

// Implementation details
extern QStringList groupsInTargetSystem();  // CreateUserJob

class GroupTests : public QObject
{
    Q_OBJECT
public:
    GroupTests();
    ~GroupTests() override {}

private Q_SLOTS:
    void initTestCase();

    void testReadGroup();
    void testCreateGroup();
};

GroupTests::GroupTests() {}

void
GroupTests::initTestCase()
{
    Logger::setupLogLevel( Logger::LOGDEBUG );
    cDebug() << "Users test started.";
    if ( !Calamares::JobQueue::instance() )
    {
        (void)new Calamares::JobQueue();
    }
    Calamares::JobQueue::instance()->globalStorage()->insert( "rootMountPoint", "/" );
}

void
GroupTests::testReadGroup()
{
    // Get the groups in the host system
    QStringList groups = groupsInTargetSystem();
    QVERIFY( groups.count() > 2 );
#ifdef __FreeBSD__
    QVERIFY( groups.contains( QStringLiteral( "wheel" ) ) );
#else
    QVERIFY( groups.contains( QStringLiteral( "root" ) ) );
#endif
    // openSUSE doesn't have "sys"
    // QVERIFY( groups.contains( QStringLiteral( "sys" ) ) );
    QVERIFY( groups.contains( QStringLiteral( "nogroup" ) ) );
    QVERIFY( groups.contains( QStringLiteral( "tty" ) ) );

    for ( const QString& s : groups )
    {
        QVERIFY( !s.isEmpty() );
        QVERIFY( !s.contains( '#' ) );
    }
}

void
GroupTests::testCreateGroup()
{
    // BUILD_AS_TEST is the source-directory path
    QFile fi( QString( "%1/tests/5-issue-1523.conf" ).arg( BUILD_AS_TEST ) );
    QVERIFY( fi.exists() );

    bool ok = false;
    const auto map = CalamaresUtils::loadYaml( fi, &ok );
    QVERIFY( ok );
    QVERIFY( map.count() > 0 );  // Just that it loaded, one key *defaultGroups*

    Config c;
    c.setConfigurationMap( map );

    QCOMPARE( c.defaultGroups().count(), 4 );
    QVERIFY( c.defaultGroups().contains( QStringLiteral( "adm" ) ) );
    QVERIFY( c.defaultGroups().contains( QStringLiteral( "bar" ) ) );

    Calamares::JobQueue::instance()->globalStorage()->insert( "rootMountPoint", "/" );

    SetupGroupsJob j( &c );
    QVERIFY( !j.exec() );  // running as regular user this should fail
}


QTEST_GUILESS_MAIN( GroupTests )

#include "utils/moc-warnings.h"

#include "TestGroupInformation.moc"
