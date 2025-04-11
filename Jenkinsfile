def withDockerNetwork(Closure inner) {
    try {
        networkId = UUID.randomUUID().toString()
        sh "docker network create ${networkId}"
        inner.call(networkId)
    } finally {
        sh "docker network rm ${networkId}"
    }
}

properties([gitLabConnection('GitLab')])

pipeline {
    agent {
        label 'docker'
    }
    environment {
        DOCKER_BUILDKIT = 1
        UID = sh(script: 'id -u', returnStdout: true).trim()
        GID = sh(script: 'id -g', returnStdout: true).trim()
        TMPDIR = '/tmp'
    }
    options {
        gitlabBuilds(builds: [
            'Build',
            'Build Doc',
            'Analyse: Lint',
            'Test: utests',
            'Test: smoketests',
            'Test: integration',
        ])
        buildDiscarder(logRotator(numToKeepStr: '4'))
        disableConcurrentBuilds()
    }
    stages {
        stage('Setup') {
            steps {
                sh '''#!/bin/bash -xe
                git clean -xdff
                '''
            }
        }
        stage('Build and Test') {
            agent {
                    dockerfile {
                        dir 'ci'
                        reuseNode true
                        additionalBuildArgs " \
                            --progress=plain \
                            --build-arg REPO=amd64 \
                            --build-arg USER=jenkins \
                            --build-arg UID=${UID} \
                            --build-arg GID=${GID} \
                            --build-arg UBUNTU_RELEASE=noble"
                        args "--privileged \
                            -v /home/jenkins/.ssh:/home/jenkins/.ssh \
                            -e HOME=/home/jenkins"
                        reuseNode true
                    }
            }
                stages {
                    stage('Build') {
                        steps {
                            gitlabCommitStatus("${STAGE_NAME}") {
                                sh '''#!/bin/bash -xe
                                ci/build.sh
                                ci/build.sh Debug --asan
                                '''
                            }
                        }
                    }
                    stage('Analyse: Lint') {
                        steps {
                            gitlabCommitStatus("${STAGE_NAME}") {
                                sh '''#!/bin/bash -xe
                                ci/clang-tidy.sh
                                ci/lint-scripts.sh
                                ci/format-code.sh --check
                                ci/lint-commits.sh origin/integration
                                ci/readme-toc.sh
                                '''
                            }
                        }
                    }
                    stage('Test: utests') {
                        steps {
                            gitlabCommitStatus("${STAGE_NAME}") {
                                sh '''#!/bin/bash -xe
                                ci/run-utests.sh
                                ci/run-utests.sh Debug
                                '''
                            }
                        }
                    }
                    stage('Test: smoketests') {
                        steps {
                            gitlabCommitStatus("${STAGE_NAME}") {
                                sh '''#!/bin/bash -xe
                                ci/run-smoketests.sh
                                ci/run-smoketests.sh Debug
                                '''
                            }
                        }
                    }
                }
        }
        stage('Setup Doc') {
            agent {
                dockerfile {
                    dir 'ci'
                    reuseNode true
                    additionalBuildArgs " \
                        --progress=plain \
                        --build-arg REPO=amd64 \
                        --build-arg USER=jenkins \
                        --build-arg UID=${UID} \
                        --build-arg GID=${GID} \
                        --build-arg UBUNTU_RELEASE=noble"
                    args "--privileged \
                        -v /home/jenkins/.ssh:/home/jenkins/.ssh \
                        -e HOME=/home/jenkins"
                    reuseNode true
                }
            }
            stages {
                stage('Build Doc') {
                    steps {
                        gitlabCommitStatus("${STAGE_NAME}") {
                            sh '''#!/bin/bash -xe
                    ci/build_doc.sh
                    '''
                        }
                    }
                }
            }
        }
        stage('Target tests') {
            environment {
                DOCKER_BUILDKIT = 1
                BUILD_ARG = '--build-arg USER=jenkins'
            }
            stages {
                stage('Test: integration') {
                    steps {
                        sshagent(credentials: ['jenkins-e2data']) {
                            gitlabCommitStatus("${STAGE_NAME}") {
                                sh '''#!/bin/bash -xe
                                    ci/run-integration-tests.sh
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

