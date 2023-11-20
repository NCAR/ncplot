pipeline {
  agent {
     node {
        label 'CentOS9'
        }
  }
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
      emailext to: "cjw@ucar.edu janine@ucar.edu cdewerd@ucar.edu",
      subject: "Jenkinsfile ncplot build failed",
      body: "See console output attached",
      attachLog: true
    }
  }
  options {
    buildDiscarder(logRotator(numToKeepStr: '10'))
  }
}
