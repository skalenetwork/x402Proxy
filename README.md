# x402Proxy

Install vcpkg

```
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
``` 

```
./vcpkg install folly wangle proxygen
```


Run

```
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=external/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_FEATURE_FLAGS=manifests 
```

## How x402proxy Reverse Proxy Works
Imagine you want to visit a website, but instead of talking to the website directly, you send your request to a helper called x402proxy. This helper stands in the middle between you and the website.

Here's what happens step by step:

1. **You make a request:**  
   You ask for a webpage or some information.

2. **x402proxy receives your request:**  
   Instead of going straight to the website, your request first goes to x402proxy.

3. **x402proxy forwards your request:**  
   x402proxy takes your request and sends it to the real website on your behalf.

4. **The website responds:**  
   The real website sends the information back to x402proxy.

5. **x402proxy sends the response to you:**  
   x402proxy takes the website's response and passes it back to you.

**Why use x402proxy?**
- It can add security by hiding the real website from you.
- It can filter or modify requests and responses.
- It can help balance traffic if many people are visiting the website at once.

In short, x402proxy is like a helpful middleman that manages communication between you and the website.
