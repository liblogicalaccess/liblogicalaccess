@Library("islog-helper") _

pipeline {
    agent none

    options {
        gitLabConnection('Gitlab Pontos')
        disableConcurrentBuilds()
        lock label: 'CONAN_CONFIGURATION_LOCK', quantity: 1
    }

    environment {
        LINUX_DOCKER_IMAGE_NAME = 'docker-registry.islog.com:5000/conan-recipes-support:latest'
        ANDROID_DOCKER_IMAGE_NAME = 'docker-registry.islog.com:5000/conan-recipes-support-android:latest'
        PACKAGE_NAME = "LogicalAccess/2.2.0@islog/${BRANCH_NAME}"

        // This is needed because MSBuild fails spuriously quiet often
        // on the build machine.
        MSBUILDDISABLENODEREUSE = 1

        // Enable recipe/package revision support for Conan.
        CONAN_REVISIONS_ENABLED = 1
    }

    parameters {
        booleanParam(name: 'BUILD_DEBUG',
                defaultValue: true,
                description: 'Perform DEBUG build')
        booleanParam(name: 'BUILD_RELEASE',
                defaultValue: true,
                description: 'Perform RELEASE build')
        booleanParam(name: 'BUILD_WINDOWS',
                defaultValue: true,
                description: 'Perform Windows build')
        booleanParam(name: 'BUILD_LINUX',
                defaultValue: true,
                description: 'Perform Linux build')
        booleanParam(name: 'BUILD_ANDROID',
                defaultValue: true,
                description: 'Perform Android build')
        string(name: 'BUILD_SINGLE_PROFILE',
                defaultValue: '',
                description: 'If not empty, will only build LLA for the specified profile.' +
                        ' This takes precedence over other parameters.')
    }

    stages {
        stage('Android') {
            when {
                beforeAgent true
                expression { params.BUILD_ANDROID && params.BUILD_SINGLE_PROFILE == '' }
            }
            steps {
                script {
                    lla.startJobForProfiles(['lla/arm_clang_android_21_debug',
                                             'lla/arm_clang_android_21',
                                             'lla/x86_clang_android_21',
                                             'lla/x64_clang_android_21',
                                             'lla/arm64_clang_android_21'
                    ])
                }
            }
        }

        stage('Minimal Feature Build') {
            when {
                beforeAgent true
                expression { params.BUILD_SINGLE_PROFILE == '' }
            }
            steps {
                script {
                    lst = []
                    if (params.BUILD_WINDOWS) {
                        if (params.BUILD_DEBUG) {
                            lst += 'lla/x64_msvc_debug_min'
                            lst += 'lla/x86_msvc_debug_min'
                        }
                        if (params.BUILD_RELEASE) {
                            lst += 'lla/x64_msvc_release_min'
                            lst += 'lla/x86_msvc_release_min'
                        }
                    }
                    if (params.BUILD_LINUX) {
                        if (params.BUILD_DEBUG) {
                            lst += 'lla/x64_gcc6_debug_min'
                        }
                        if (params.BUILD_RELEASE) {
                            lst += 'lla/x64_gcc6_release_min'
                        }
                    }
                    lla.startJobForProfiles(lst)
                }
            }
        }

        stage('Complete Feature Build') {
            when {
                beforeAgent true
                expression { params.BUILD_SINGLE_PROFILE == '' }
            }
            steps {
                script {
                    lst = []
                    if (params.BUILD_WINDOWS) {
                        if (params.BUILD_DEBUG) {
                            lst += 'lla/x64_msvc_debug_full'
                            lst += 'lla/x86_msvc_debug_full'
                        }
                        if (params.BUILD_RELEASE) {
                            lst += 'lla/x64_msvc_release_full'
                            lst += 'lla/x86_msvc_release_full'
                        }
                    }
                    if (params.BUILD_LINUX) {
                        if (params.BUILD_DEBUG) {
                            lst += 'lla/x64_gcc6_debug_full'
                        }
                        if (params.BUILD_RELEASE) {
                            lst += 'lla/x64_gcc6_release_full'
                        }
                    }
                    lla.startJobForProfiles(lst)
                }
            }
        }

        stage('Single profile') {
            when {
                beforeAgent true
                expression { params.BUILD_SINGLE_PROFILE != '' }
            }
            steps {
                script {
                    lla.startJobForProfiles([params.BUILD_SINGLE_PROFILE])
                }
            }
        }
    }

    post {
        changed {
            script {
                if (currentBuild.currentResult == 'FAILURE' || currentBuild.currentResult == 'SUCCESS') {
                    // Other values: SUCCESS, UNSTABLE
                    // Send an email only if the build status has changed from green/unstable to red
                    emailext subject: '$DEFAULT_SUBJECT',
                            body: '$DEFAULT_CONTENT',
                            recipientProviders: [
                                    [$class: 'CulpritsRecipientProvider'],
                                    [$class: 'DevelopersRecipientProvider'],
                                    [$class: 'RequesterRecipientProvider']
                            ],
                            replyTo: 'cis@islog.com',
                            to: 'reports@islog.com'
                }
            }
        }
    }
}

def debPackageBuild() {
    sh './scripts/debian/git-buildpackage.sh'
    sh 'cd /home/jenkins/liblogicalaccess-debian && dupload --nomail ../*.changes'
}
