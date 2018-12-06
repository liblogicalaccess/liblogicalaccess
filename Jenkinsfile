@Library("islog-helper") _

pipeline {
    agent none

    options {
        gitLabConnection('Gitlab Pontos')
        disableConcurrentBuilds()
        lock label: 'CONAN_CONFIGURATION_LOCK', quantity: 1
    }

    environment {
        LINUX_DOCKER_IMAGE_NAME = 'docker-registry.islog.com:5000/conan-recipes-support:cis-latest'
        ANDROID_DOCKER_IMAGE_NAME = 'docker-registry.islog.com:5000/conan-recipes-support-android:cis-latest'
        PACKAGE_NAME = "LogicalAccess/2.1.0@islog/${BRANCH_NAME}"

        // This is needed because MSBuild fails spuriously quiet often
        // on the build machine.
        MSBUILDDISABLENODEREUSE = 1
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
    }

    /**
     * This Jenkinsfile driven build will generate Conan package for multiple
     * combination of settings and options.
     */

    stages {
        stage('Android') {
            when { expression { params.BUILD_ANDROID } }
            steps {
                script {
                    // Test to see how parallel that is
                    lla.startJobForProfiles(['lla/arm_clang_android_21_debug'])
                    lla.startJobForProfiles(['lla/arm_clang_android_21'])
                    lla.startJobForProfiles(['lla/x86_clang_android_21'])
                    
                    //lla.startJobForProfiles(['lla/arm_clang_android_21_debug',
                    //                         'lla/arm_clang_android_21',
                    //                         'lla/x86_clang_android_21'])
                }
            }
        }

        stage('Minimal Feature Build') {
            steps {
                script {
                    lla.startJobForProfiles(["lla/x64_gcc6_release_min",
                                             "lla/x64_gcc6_debug_min",
                                             "lla/x64_msvc_release_min",
                                             "lla/x64_msvc_debug_min",
                                             "lla/x86_msvc_release_min",
                                             "lla/x86_msvc_debug_min"])
                }
            }
        }

        stage('Complete Feature Build') {
            steps {
                script {
                    lla.startJobForProfiles(["lla/x64_gcc6_release_full",
                                             "lla/x64_gcc6_debug_full",
                                             "lla/x64_msvc_release_full",
                                             "lla/x64_msvc_debug_full",
                                             "lla/x86_msvc_release_full",
                                             "lla/x86_msvc_debug_full"])
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
