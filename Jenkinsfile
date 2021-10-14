def UID
def GID

node {
    UID = sh(script: 'id -u', returnStdout: true).trim()
    GID = sh(script: 'id -g', returnStdout: true).trim()
}

pipeline {
    agent any
    options {
        buildDiscarder(logRotator(numToKeepStr: '4'))
        disableConcurrentBuilds()
    }
    stages {
        stage ('Build target') {
            agent {
                dockerfile {
                    dir 'ci'
                    reuseNode true
                    additionalBuildArgs "--build-arg USER=jenkins \
                        --build-arg UID=${UID} --build-arg GID=${GID}"
                    args "--privileged \
                        -v /home/jenkins/.ssh:/home/jenkins/.ssh \
                        -e HOME=/home/jenkins"
                }
            }
            steps {
                sh '''#!/bin/bash -xe
                  env
                  TMPDIR=
                  PARALLEL_MAKE="-j 1"

                  git clean -xdff

                  ci/build.sh
                  rm -rf build
                '''
            }
        }
        stage('Store result') {
            steps {
                archiveArtifacts 'result/**'
            }
        }
    }
}

