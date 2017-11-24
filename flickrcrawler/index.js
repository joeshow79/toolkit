var Flickr = require('flickr-sdk')
var fs = require('fs')
var schedule = require('node-schedule');

var writeFile = require('write-file')
var mkdirp = require('mkdirp');

var axios = require('axios')
var tools = require('./tools')
var caiji = require('./tools').caiji();



process.env.FLICKR_API_KEY = 'bad9db564dc9ca07de65f72b7015e4cf'
process.env.FLICKR_API_SECRET = 'c1829f0ff0145847'


var args = process.argv.splice(2)
var page = (args.length > 0) ? Number(args[0]) : 1
var keywords = args[1] != null ? String(args[1]) : null;
if(keywords == null){
  console.log('请设置搜索关键字')
  process.exit()
}
// console.log(page)

var flickr = new Flickr(process.env.FLICKR_API_KEY);

function saveImage(link, savepath) {
  if (fs.existsSync(savepath)) {
    // console.log('文件已经存在: ' + savepath)
  }else{
    tools.getRemoteImage(link).then(function (res) {
      if(res.bits == null){
        console.error('没有获取图片内容');
        return false;
      }else{
        // console.log('成功下载图片: '+ link)
        let content = res.bits;
        fs.exists(savepath, (exists) => {
          if (exists) {
            console.error('file already exists');
            return false;
          } else {
            fs.writeFile(savepath, content, function (err) {
              if (err) {
                return console.error(err)
                return false;
              }else{
                fs.chmodSync(savepath, '777')
                console.log('file is written: '+ savepath)
                return true;
              }
            });
          }
        });
      }
    }).catch(function (error) {
      console.error(error);
    });
  }
}

function getPageImage(list) {
  // console.log(res.body)
  for(let i =0;i<list.length;i++){
    // console.log(list[i]);
    let link = 'https://farm{farm-id}.staticflickr.com/{server-id}/{id}_{secret}.jpg'
    link = link.replace('{farm-id}', list[i].farm)
      .replace('{server-id}', list[i].server)
      .replace('{id}', list[i].id)
      .replace('{secret}', list[i].secret);

    let dir = './download/' + keywords + '/farm' + list[i].farm + '.staticflickr.com/' + list[i].server + '/';
    let file = list[i].id + '_' +list[i].secret+ '.jpg';

    // console.log(dir)
    if (fs.existsSync(dir)) {
      // console.log('目录已经创建: ' + dir)
      saveImage(link, dir + file)
    }else{
      mkdirp(dir, function (err) {
        if (err) console.error(err)
        else {
          console.log('成功创建目录: ' + dir)
          saveImage(link, dir + file)
        }
      });
    }
  }
}

function getFlickrPage(keywords, page) {
  flickr.photos.search({
    text: keywords,
    page: page
  }).then(function (res) {
    if(res.body.stat == 'ok' && res.body.photos.photo != null) {
      console.log('采集整理: [keywords: '+ keywords + ', page: ' + page + ']')
      let list = res.body.photos.photo
      getPageImage(list)
      // fs.writeFileSync('./'+page+'.txt', JSON.stringify(list))
    }
  }).catch(function (err) {
    console.error('getFlickrPage: ', err);
  });
}

flickr.photos.search({
  text: keywords
}).then(function (res) {
  console.log('采集整理: [keywords: '+ keywords + ', page: ' + page + ']')
  if(res.body.stat == 'ok' && res.body.photos.photo != null){
    let list = res.body.photos.photo
    getPageImage(list)

    // 存储采集任务
    var cron = []
    for(page++;page<=res.body.photos.pages;page++){
      cron.push({keywords: keywords, page: page, pages: res.body.photos.pages})
    }

    // 每分钟执行一次翻页采集
    var rule = new schedule.RecurrenceRule();
    rule.second = [0, 6, 12, 18, 24, 30, 36, 42, 48, 54]
    var j = schedule.scheduleJob(rule, function(){
      console.log(new Date());
      let funArgs = cron.shift()
      if(funArgs != null){
        // console.log(funArgs)
        getFlickrPage(funArgs.keywords, funArgs.page)
      }else{
        console.log('采集完成!')
        process.exit()
      }
    });
  }
}).catch(function (err) {
  console.error('flickr.photos.search', err);
});
