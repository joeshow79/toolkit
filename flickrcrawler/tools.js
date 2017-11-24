var axios = require('axios')
var fs = require('fs')
var path = require("path");


var Promise = require('bluebird')
var URL = require('url')
const fileType = require('file-type')
const http = require('http')
const https = require('https')
const urlobj = require('url');
const dns = require('dns');

var tools = {};

var caiji = null
tools.caiji = function () {
  if(caiji == null){
    let config = {
      timeout: 30000,
      headers: {
        'User-Agent': 'Mozilla/5.0 (Windows NT 6.1; WOW64; rv:54.0) Gecko/20100101 Firefox/54.0'
      }
    }
    caiji = axios.create(config)
    return caiji
  }else{
    return caiji
  }
}

tools.getRemoteImage = function (url) {
  return new Promise(function (resolve, reject) {
    var cut_url = urlobj.parse(url)
    // console.log(cut_url)
    const options = {
      family: 4,
      hints: dns.ADDRCONFIG | dns.V4MAPPED,
      all: true
    };
    dns.lookup(cut_url.hostname, options, (err, addresses) => {
      if (err) {
        return reject(err)
      } else {
        // console.log('addresses: %j', addresses);

        var p = URL.parse(url)
        var request = {}
        if (p['protocol'] === 'http:') {
          // console.log('http')
          request = require('http')
        }
        if (p['protocol'] === 'https:') {
          // console.log('https')
          request = require('https')
        }
        url = encodeURI(url)
        // console.log(request);return;
        try {
          request.get(url, function (res) {
            var body = ''
            res.on('data', function (chunk) {
              if (res.statusCode == 200) {
                body += chunk
              }
            })

            res.setEncoding('binary')
            res.on('end', function () {
              var type = {}
              var bits = ''
              var fileBinary = new Buffer(body, 'binary')
              bits = fileBinary.toString('base64')
              type = fileType(fileBinary)
              var data = {
                type: type,
                bits: fileBinary,
                body: body
              }
              return resolve(data)
            })
          })
        } catch (err) {
          // console.log(err)
          return reject(err)
        }
      }
    })
  })
}

module.exports = tools;