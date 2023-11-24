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
        label 'agent01'
    }
    environment {
        DOCKER_BUILDKIT=1
        UID = sh(script: 'id -u', returnStdout: true).trim()
        GID = sh(script: 'id -g', returnStdout: true).trim()
        TMPDIR = '/tmp'
    }
    options {
        gitlabBuilds(builds: [
            "Build (amd64)",
            "Build (arm64v8)",
            "Package (amd64)",
            "Package (arm64v8)",
            "Build Doc(amd64)",
            "Analyse: Lint (amd64)",
            "Analyse: Lint (arm64v8)",
            "Test: utests (amd64)",
            "Test: utests (arm64v8)",
            "Test: smoketests (amd64)",
            "Test: smoketests (arm64v8)",
            "Test: integration",
            "Demo (amd64)",
            "Demo (arm64v8)"
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
        stage ('Build and Test') {
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
                        additionalBuildArgs " \
                            --ssh default=/root/.ssh/id_ed25519 \
                            --progress=plain \
                            --build-arg REPO=${ARCH} \
                            --build-arg USER=jenkins \
                            --build-arg UID=${UID} \
                            --build-arg GID=${GID} \
                            --build-arg UBUNTU_RELEASE=jammy"
                        args "--privileged \
                            -v /home/jenkins/.ssh:/home/jenkins/.ssh \
                            -e HOME=/home/jenkins"
                        reuseNode true
                    }
                }
                stages {
                    stage ('Build') {
                        steps {
                            gitlabCommitStatus("${STAGE_NAME} (${ARCH})") {
                                sh '''#!/bin/bash -xe
                                ci/build.sh
                                ci/build.sh Debug --asan
                                '''
                            }
                        }
                    }
                    stage ('Package') {
                        steps {
                            gitlabCommitStatus("${STAGE_NAME} (${ARCH})") {
                                sh '''#!/bin/bash -xe
                                ci/package.sh
                                '''
                            }
                        }
                    }
		    stage ('Build Doc') {
		        steps {
			    gitlabCommitStatus("${STAGE_NAME} (${ARCH})") {
			        sh '''#!/bin/bash -xe
			        ci/build_doc.sh
			        '''
			    }
		        }
	            }
                    stage ('Analyse: Lint') {
                        steps {
                            gitlabCommitStatus("${STAGE_NAME} (${ARCH})") {
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
                            gitlabCommitStatus("${STAGE_NAME} (${ARCH})") {
                                sh '''#!/bin/bash -xe
                                ci/run-utests.sh
                                ci/run-utests.sh Debug
                                '''
                            }
                        }
                    }
                    stage ('Test: smoketests') {
                        steps {
                            gitlabCommitStatus("${STAGE_NAME} (${ARCH})") {
                                sh '''#!/bin/bash -xe
                                ci/run-smoketests.sh
                                ci/run-smoketests.sh Debug
                                '''
                            }
                        }
                    }
                    stage ('Demo') {
                        steps {
                            gitlabCommitStatus("${STAGE_NAME} (${ARCH})") {
                                sh '''#!/bin/bash -xe
                                ci/demo.sh 2>&1 | tee result/demo_output.txt
                                '''
                            }
                        }
                    }
                }
            }
        }
        stage ('Target tests') {
            environment {
                DOCKER_BUILDKIT = 1
                BUILD_ARG = "--build-arg USER=jenkins"
            }
            stages {
                stage ('Test: integration') {
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
            archiveArtifacts 'result/**, build/**'
        }
    }
}
