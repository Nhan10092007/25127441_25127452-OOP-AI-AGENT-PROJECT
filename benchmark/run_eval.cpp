#include"harness/harness_runner.h"
#include<curl/curl.h>
#include<stdexcept>
#include<iostream>

int main(){
    curl_global_init(CURL_GLOBAL_ALL);
    try{
        HarnessRunner runner("./config/config.json", "./skills", "./benchmark/tasks.json");
        runner.runBatch();
    }
    catch(const std::exception& e){
        std::cerr << "Error during run batch" << e.what() << "\n";
    }
    curl_global_cleanup();
    return 0;
}