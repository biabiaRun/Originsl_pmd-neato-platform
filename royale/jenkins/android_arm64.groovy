@Library('pipelib@v0.3') _

/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/


// Do not remove this variables, because they are also read and used by the
// startDroidEnv.sh script. This ensures that we don't need to configure the
// docker image twice.
def dockerRegistry='depmdlx06.intra.ifm:5000'
def dockerImage='royaleandroid_x8664:0.1'

void compileAndroidArm64 (Map config)
{
    node(config.agent)
    {
        docker.withRegistry(config.registry, config.regCredentials) {
            def enviro = []
            if (params.GLOBAL_BUILD_NUMBER?.trim()) {
                enviro.add("ROYALE_VERSION_BUILD=${params.GLOBAL_BUILD_NUMBER.trim()}")
            }
            if (params.JENKINS_BUILD_TYPE?.trim()) {
                enviro.add("JENKINS_BUILD_TYPE=${params.JENKINS_BUILD_TYPE.trim()}")
            }
            buildNo = params.GLOBAL_BUILD_NUMBER
            buildimg = docker.image(config.image)
            buildimg.pull()
            buildimg.inside {
                withEnv(enviro) {
                stage(config.name + ' git')
                {
                    dir ('royal')
                    {
                        config.coCommand.call();
                    }
                    dir ('licenses')
                    {
                        checkout (
                            poll: false,
                            scm:
                            [
                                $class: 'GitSCM',
                                branches: [[name: '*/master']],
                                doGenerateSubmoduleConfigurations: false,
                                extensions: [],
                                gitTool: 'Default',
                                submoduleCfg: [],
                                userRemoteConfigs: [[
                                    credentialsId: 'licenses_git_key',
                                    url: 'ssh://git@dettgit.intra.ifm:7999/moon/license-texts.git'
                                ]]
                            ]
                        )
                    }
                }
                stage(config.name + ' Configure')
                {
                    removeDir('build');
                    dir ('build')
                    {
                        sh script: "../royal/jenkins/build_android.sh --bitsize 64 configure"
                    }
                }
                stage(config.name + ' Build')
                {
                    dir ('build')
                    {
                        sh script: '../royal/jenkins/build_android.sh --verbose build'
                    }
                }
                stage(config.name + ' Package')
                {
                    dir ('build')
                    {
                        sh script: '../royal/jenkins/build_android.sh --verbose package'
                        fingerprint '_CPack_Packages/Android/ZIP/libroyale-*-ANDROID-arm-*Bit/bin/**,_CPack_Packages/Android/ZIP/libroyale-*-ANDROID-arm-*Bit/include/**,_CPack_Packages/Android/ZIP/libroyale-*-ANDROID-arm-*Bit/samples/**,_CPack_Packages/Android/ZIP/libroyale-*-ANDROID-arm-*Bit/*.pdf,_CPack_Packages/Android/ZIP/libroyale-*-ANDROID-arm-*Bit/*.txt,_CPack_Packages/Android/ZIP/libroyale-*-ANDROID-arm-*Bit/*.md,packages/**'
                        config.storeCommand.call()
                    }
                    warnings canComputeNew: false, canResolveRelativePaths: false, categoriesPattern: '', consoleParsers: [[parserName: 'GNU Make + GNU C Compiler (gcc)']], defaultEncoding: '', excludePattern: '', healthy: '', includePattern: '', messagesPattern: '', unHealthy: ''
                }
            }
            }
        }
    }
}
try {
    properties([parameters([
        string(defaultValue: "${scm.branches[0].name}", description: 'The branch that should be built', name: 'BRANCH'),
        string(defaultValue: '/var/www/packages/tmp', description: 'The copy location for the package', name: 'PKG_COPY_LOCATION'),
        string(defaultValue: '', description: 'The global build number', name: 'GLOBAL_BUILD_NUMBER'),
        string(defaultValue: '', description: 'The current build type', name: 'JENKINS_BUILD_TYPE'),
        string(defaultValue: '', description: 'Who should receive error emails', name: 'ROYALE_EMAIL_RECIP'),
        string(defaultValue: '', description: 'location of the license file', name: 'ROYALE_LICENSE_FILE'),
        string(defaultValue: '', description: 'suffix for customer special version', name: 'ROYALE_CUSTOMER_SUFFIX')]),
        pipelineTriggers([])
    ])

    timestamps {
        compileAndroidArm64 (
            agent: 'DOCKER_64',
            name: 'Android Arm 64Bit',
            registry: 'https://' + dockerRegistry,
            regCredentials: 'DOCKER_REGISTRY',
            image: dockerImage,
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
                    sshPublisher(publishers: [sshPublisherDesc(configName: 'Ubuntu 14.04 Webserver', transfers: [sshTransfer(excludes: '', execCommand: '', execTimeout: 120000, flatten: false, makeEmptyDirs: false, noDefaultExcludes: false, patternSeparator: '[, ]+', remoteDirectory: params.PKG_COPY_LOCATION, remoteDirectorySDF: false, removePrefix: '', sourceFiles: '**')], usePromotionTimestamp: false, useWorkspaceInPromotion: false, verbose: false)])
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
