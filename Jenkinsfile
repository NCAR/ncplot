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
    success {
      mail(body: 'ncplot Jenkinsfile build successful', subject: 'ncplot Jenkinsfile build successful', to: 'cjw@ucar.edu taylort@ucar.edu')
    }
  }
  options {
    buildDiscarder(logRotator(numToKeepStr: '10'))
  }
}
