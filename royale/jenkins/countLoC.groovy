/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

@Library('pipelib@v0.5') _

try {
    properties([parameters([
        string(
          defaultValue: "${scm.branches[0].name}",
          description: 'The branch that should be built',
          name: 'BRANCH'
        ),
        string(
          defaultValue: '/var/www/reports',
          description: 'The copy location for the reports',
          name: 'REPORT_COPY_LOCATION'
        )]),
        pipelineTriggers([])
    ])

    timestamps {
        reportdir = 'countReport'
        countLinesOfCode (
            project: 'Royale',
            owncode: 'README.md CMakeLists.txt jenkins samples source '
                     + 'spectre cmake doc firmware scripts test tools',
            extcode: 'contrib drivers',
            remcode: 'contrib/gtest',
            bincode: 'contrib/deployQt5 contrib/qt-android-cmake-master '
                     + 'contrib/libusb contrib/numpy drivers',
            reportname: 'jenkins-report.html',
            reportdirectory: reportdir,
            coCommand: {
                checkout( [$class: 'GitSCM',
                    branches: [[name: "${params.BRANCH}"]],
                    browser: [
                      $class: 'BitbucketWeb',
                      repoUrl: 'https://git.ifm.com/projects/ROYALE/repos/royale/'
                    ],
                    doGenerateSubmoduleConfigurations: false,
                    extensions: [],
                    gitTool: 'Default',
                    submoduleCfg: [],
                    userRemoteConfigs: [[
                      credentialsId: 'royale_git_key',
                      url: 'ssh://git@git.ifm.com:7999/royale/royale.git'
                    ]]
                ])
            },
            storeCommand: {
                dir (reportdir)
                {
                    uploadFiles (
                        sourceFiles: 'report-royale-*.html',
                        remoteDirectory: params.REPORT_COPY_LOCATION
                    )
                }
            }
        )
    }
}
catch (e)
{
    currentBuild.result = "FAILURE"
    throw e
}
