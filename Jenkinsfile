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
			agent {	docker { image 'ubuntu-64-zesty-build' } }
			steps {
				debPackageBuild()
			}
		}
    }
}

def debPackageBuild() {
	sh './scripts/debian/git-buildpackage.sh'
	sh 'cd /home/jenkins/liblogicalaccess-debian && dupload --nomail ../*.changes'
}
