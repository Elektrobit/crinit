pipeline {
    agent {
        label 'agent'
    }
    environment {
        UID = sh(script: 'id -u', returnStdout: true).trim()
        GID = sh(script: 'id -g', returnStdout: true).trim()
    }
    options {
        buildDiscarder(logRotator(numToKeepStr: '4'))
        disableConcurrentBuilds()
    }
    stages {
        stage ('Setup') {
            steps {
                sh '''#!/bin/bash -xe
                git clean -xdff
                '''
            }
        }
        stage ('Build') {
            matrix {
                axes {
                    axis {
                        name 'ARCH'
                        values 'amd64', 'arm64v8'
                    }
                }
                agent {
                    dockerfile {
                        dir 'ci'
                        reuseNode true
                        additionalBuildArgs "--build-arg REPO=${ARCH} --build-arg USER=jenkins \
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
                            ci/build.sh
                            '''
                        }
                    }
                    stage ('Analyse: Lint') {
                        steps {
                            sh '''#!/bin/bash -xe
                            ci/clang-tidy.sh
                            '''
                            sh '''#!/bin/bash -xe
                            ci/checkversion.sh
                            '''
                        }
                    }
                    stage ('Analyse: Unit Tests') {
                        steps {
                            sh '''#!/bin/bash -xe
                            ci/run-utests.sh
                            '''
                        }
                    }
                    stage ('Test: smoketests') {
                        steps {
                            sh '''#!/bin/bash -xe
                            ci/run-smoketests.sh Release --valgrind
                            '''
                        }
                    }
                    stage ('Demo') {
                        steps {
                            sh '''#!/bin/bash -xe
                            ci/demo.sh 2>&1 | tee result/demo_output.txt
                            '''
                        }
                    }
                }
            }
        }
        stage ('Post') {
            stages {
                stage('Store result') {
                    steps {
                        archiveArtifacts 'result/**'
                    }
                }
            }
        }
    }
}
