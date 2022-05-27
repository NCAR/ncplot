pipeline {
  agent any
  triggers {
    pollSCM('H H * * *')
  }
  stages {
    stage('Checkout scm') {
      steps {
        git 'eolJenkins:ncar/ncplot'
      }
    }
    stage('Build') {
      steps {
        sh 'make'
      }
    }
  }
  post {
    failure {
      mail(body: 'ncplot Jenkinsfile build failed', subject: 'ncplot Jenkinsfile build failed', to: 'cjw@ucar.edu taylort@ucar.edu')
    }
  }
  options {
    buildDiscarder(logRotator(numToKeepStr: '10'))
  }
}
