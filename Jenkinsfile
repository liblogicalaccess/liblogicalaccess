pipeline {
	agent none
    stages {
		stage('Debian build') {
			agent {	docker { image 'debian-64-stable-build'	} }
			steps {
				debPackageBuild()
			}
		}
		
		stage('Ubuntu build') {
			agent {	docker { image 'ubuntu-64-stable-bionic' } }
			steps {
				debPackageBuild()
			}
		}

		stage('Rasbian build') {
			agent {	docker { image 'docker-registry.islog.com:5000/raspbian-lite' } }
			steps {
				debPackageBuild()
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

def debPackageBuild() {
	sh './scripts/debian/git-buildpackage.sh'
	sh 'cd /home/jenkins/liblogicalaccess-debian && dupload --nomail ../*.changes'
}
