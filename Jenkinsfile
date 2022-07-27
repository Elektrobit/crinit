properties([gitLabConnection('GitLab')])

pipeline {
    agent {
        label 'agent'
    }
    environment {
        UID = sh(script: 'id -u', returnStdout: true).trim()
        GID = sh(script: 'id -g', returnStdout: true).trim()
        TMPDIR = '/tmp'
    }
    options {
        gitlabBuilds(builds: [
            "Build",
            "Analyse: Lint",
            "Test: utests",
            "Test: smoketests",
            "Demo"
        ])
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
                            gitlabCommitStatus("Build") {
                                sh '''#!/bin/bash -xe
                                ci/build.sh
                                ci/build.sh Debug
                                '''
                            }
                        }
                    }
                    stage ('Analyse: Lint') {
                        steps {
                            gitlabCommitStatus("Analyse: Lint") {
                                sh '''#!/bin/bash -xe
                                ci/clang-tidy.sh
                                '''
                                sh '''#!/bin/bash -xe
                                ci/checkversion.sh
                                '''
                            }
                        }
                    }
                    stage ('Test: utests') {
                        steps {
                            gitlabCommitStatus("Test: utests") {
                                sh '''#!/bin/bash -xe
                                ci/run-utests.sh
                                ci/run-utests.sh Debug
                                '''
                            }
                        }
                    }
                    stage ('Test: smoketests') {
                        steps {
                            gitlabCommitStatus("Test: smoketests") {
                                sh '''#!/bin/bash -xe
                                ci/run-smoketests.sh
                                ci/run-smoketests.sh Debug
                                '''
                            }
                        }
                    }
                    stage ('Demo') {
                        steps {
                            gitlabCommitStatus("Demo") {
                                sh '''#!/bin/bash -xe
                                ci/demo.sh 2>&1 | tee result/demo_output.txt
                                '''
                            }
                        }
                    }
                }
            }
        }
    }
    post {
        always {
            archiveArtifacts 'result/**'
        }
    }
}
