# estimateCVRiskRESTAPI
## purpose
Using C to expose R package [estimateCVRisk](https://github.com/DGruen89/estimateCVRisk.git) to REST API. Writing this just for fun without all error handling that needed, please don't use in production environment. Other examples about using C to call R function could find [here](https://github.com/wch/r-source/tree/trunk/tests/Embedding).

## container
Building container：
```sh
sudo docker build -t [image tag] .
```

Using container：
```sh
sudo docker run -d -p [host port]:[api port] [image tag] [api port]
```

nerdctl and podman are ok too.

## simple test
API test command：
```sh
curl -X POST -d '@[payload file name with path]' -H "Content-Type: application/json" http://[IP]:[api port]
```

## DIY
If someone would like to compile this, could follow Dockerfile to setup environment.
However, C stack warning message might show, the following command could temperately set to unlimited on debian-like.
```sh
sudo prlimit --stack=unlimited --pid $$; ulimit -s unlimited
```
