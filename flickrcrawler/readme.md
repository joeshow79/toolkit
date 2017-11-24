##配置nodejs环境,并执行脚本

####安装nodejs环境

````
 curl -sL https://deb.nodesource.com/setup_8.x | sudo -E bash -
 sudo apt-get install -y nodejs
````

####安装nodejs扩展
````
cd [代码根目录]
npm install
````

####直接执行
````
node index.js [page(从第几页开始采集)] [keywords(搜索的关键字)]
````
````
示例:
node index.js 1 selfie
````

##编译打包成可执行文件

#####安装编译依赖:
````
npm install -g node-gyp pkg
````

#####安装编译工具链:
````
apt install build-essential openssl libtool libpcre3 libpcre3-dev zlib1g-dev
````
#####执行打包编译,或者直接运行代码根目录下的build.sh:
pkg . -t node8-linux-x64 -o ./bin/flickr


#####
编译成功后会直接生成打包文件./bin/flickr,可以脱离nodejs环境直接运行
下面为调用示例,会把图片下载到当前目录./download/selfie下面:
````
./bin/flickr 1 selfie
````

