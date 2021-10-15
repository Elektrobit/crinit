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
        stage ('Build and run demo') {
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
            stages {
                stage ('Build') {
                    steps {
                        sh '''#!/bin/bash -xe
                          env
                          TMPDIR=
                          PARALLEL_MAKE="-j 1"

                          git clean -xdff

                          ci/build.sh
                        '''
                    }
                }
                stage ('Demo') {
                    steps {
                        sh '''#!/bin/bash -xe
                          env
                          TMPDIR=

                          ci/demo.sh 2>&1 | tee result/demo_output.txt
                        '''
                    }
                }
            }
        }
        stage('Store result') {
            steps {
                archiveArtifacts 'result/**'
            }
        }
    }
}

