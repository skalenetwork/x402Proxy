**Why use x402proxy?**

- It instantly adds x402 payments to existing websites and web APIs.
- It is fully x402 compliant.
- It is easy to deploy and configure.
- It is a high performance C++ based implementation, supporting large number of concurrent connections.
- It supports both Base and SKALE networks.
- It supports flexible payment models and x402 options.
- It provides detailed logging and monitoring.
- It is open source and free to use.

## How x402proxy Reverse Proxy Works
Imagine you want to visit a website, but instead of talking to the website directly, you send your request to a helper website called x402proxy. 
x402proxy stands in the middle between you and the website.

x402proxy charges users for access to web resources using the x402 protocol by Coinbase.

It supports both Base and SKALE networks, enabling payments across these blockchains.




Here's what happens step by step:

1. **You make a request:**  
   You ask for a webpage or some information.

2. **x402proxy receives your request:**  
   Instead of going straight to the website, your request first goes to x402proxy. 
   It checks if it comes with the right payment. If not, it will ask you to pay before proceeding.

3. **x402proxy forwards your request:**  
   Once the payment is received, x402proxy takes your request and sends it to the real website on your behalf.

4. **The website responds:**  
   The real website sends the information back to x402proxy.

5. **x402proxy sends the response to you:**  
   x402proxy takes the website's response and passes it back to you.


# Build instructions

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



