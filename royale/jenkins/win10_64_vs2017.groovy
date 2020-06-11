@Library('pipelib@v0.3') _
try {
    def choiceValues = ['OFF', 'ON'].join('\n')
    properties([parameters([
        string(defaultValue: 'feature/sprint53', description: 'The branch that should be built', name: 'BRANCH'),
        string(defaultValue: '/var/www/packages/tmp', description: 'The copy location for the package', name: 'PKG_COPY_LOCATION'),
        string(defaultValue: '', description: 'The global build number', name: 'GLOBAL_BUILD_NUMBER'),
        string(defaultValue: '', description: 'The current build type', name: 'JENKINS_BUILD_TYPE'),
        string(defaultValue: '', description: 'Who should receive error emails', name: 'ROYALE_EMAIL_RECIP'),
        string(defaultValue: '', description: 'location of the license file', name: 'ROYALE_LICENSE_FILE'),
        string(defaultValue: '', description: '', name: 'ROYALE_CUSTOMER_SUFFIX'),
        choice(choices: choiceValues, description: 'Also build the developer package', name: 'ROYALE_DEVELOPER_EDITION'),
        choice(choices: choiceValues, description: 'Enables ZIP packaging instead of an installer', name: 'ROYALE_ZIP_PACKAGE')]),
        pipelineTriggers([])
    ])

    timestamps {
        royale.compileVs2017_64 (
            agent: 'WINDOWS10_64',
            name: 'Windows 10 64Bit',
            coCommand: {
                checkout( [$class: 'GitSCM',
                    branches: [[name: "${params.BRANCH}"]],
                    browser: [$class: 'BitbucketWeb', repoUrl: 'https://git.ifm.com/projects/ROYALE/repos/royale/'],
                    doGenerateSubmoduleConfigurations: false,
                    extensions: [],
                    gitTool: 'Default',
                    submoduleCfg: [],
                    userRemoteConfigs: [[credentialsId: 'royale_git_key', url: 'ssh://git@git.ifm.com:7999/royale/royale.git']]
                ])
            },
            storeCommand: {
                dir ('packages')
                {
                    sshPublisher(publishers: [sshPublisherDesc(configName: 'Ubuntu 14.04 Webserver', transfers: [sshTransfer(excludes: '', execCommand: '', execTimeout: 120000, flatten: false, makeEmptyDirs: false, noDefaultExcludes: false, patternSeparator: '[, ]+', remoteDirectory: params.PKG_COPY_LOCATION, remoteDirectorySDF: false, removePrefix: '', sourceFiles: '*.zip, *.exe')], usePromotionTimestamp: false, useWorkspaceInPromotion: false, verbose: false)])
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
finally
{
    sendMailIfNecessary ( recipients: params.ROYALE_EMAIL_RECIP )
}
