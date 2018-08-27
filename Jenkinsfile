pipeline {
    agent none
    stages {
        stage('Builds') {
            parallel {
                stage('Debian build') {
                    agent { docker { image 'debian-64-stable-build' } }
                    steps {
                        debPackageBuild()
                    }
                }

                stage('Ubuntu build') {
                    agent { docker { image 'ubuntu-64-stable-bionic' } }
                    steps {
                        debPackageBuild()
                    }
                }

                stage('Build With Unittests') {
                    agent { docker { image 'debian-64-stable-build' } }
                    steps {
                        installGoogleTest()
                        sh 'mkdir build && cd build && cmake -DLLA_DISABLE_IKS=1 -DLLA_BUILD_UNITTESTS=1 .. && make -j6'

                        // Run test -- Should be another stage most likely.
                        sh 'cd build/tests/unittest && for f in test* ; do GTEST_OUTPUT="xml:./" ./$f ; done'
                    }
                    post {
                        always {
                            junit 'build/tests/unittest/*.xml'
                        }
                    }
                }
            }
        }
    }

    post {
        changed {
            script {
                if (currentBuild.currentResult == 'FAILURE') { // Other values: SUCCESS, UNSTABLE
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

def installGoogleTest()
{
    sh 'sudo apt-get install libgtest-dev -y'
    sh 'cd /usr/src/gtest && sudo cmake CMakeLists.txt && sudo make && sudo cp *.a /usr/lib'
}

def debPackageBuild() {
    sh './scripts/debian/git-buildpackage.sh'
    sh 'cd /home/jenkins/liblogicalaccess-debian && dupload --nomail ../*.changes'
}
