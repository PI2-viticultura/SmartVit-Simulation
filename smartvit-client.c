#include<stdlib.h>
#include<stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

// Generic sensor
// struct sensor_data{ 
//     char sensor_id[20];
//     float value;
//     char date_time[50];
// };

// Sensors structs

struct sensor_anemometro{
    char sensor_id[20];
    float vento_RPM;
    float vento_KMH;
    float vento_MS;
};

struct sensor_biruta{
    char sensor_id[20];
    float vento_direcao;
};

struct sensor_pluviometro{
    char sensor_id[20];
    float qtd_chuva;
};

struct sensor_bme{
    char sensor_id[20];
    float temp_celsius;
    float humidity_percent;
    float pressure_hPa;
};

struct all_sensors_data{
    struct sensor_anemometro sensor_anemometro;
    struct sensor_biruta sensor_biruta;
    struct sensor_pluviometro sensor_pluviometro;
    struct sensor_bme sensor_bme;
    char date_time[50]; 
};

void error(const char *msg) { 
    perror(msg); 
    exit(0); 
}

int main(){
    // struct sensor_data data;
    struct all_sensors_data total_data;

    // Host and socket data
    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;    

    // Define request attributes
    int portno =  8000;
    char *host =  "localhost";
    char request[2048];
    char service_method [64] = "measurement";
    char data_sent [1024];    
    char message[1024],response[4096]; // data_sent and data_response
    char completed_request[6000];

    // Put data to send into struct

    // Generic struct 
    // memcpy(data.sensor_id, "1234586564", 20);
    // data.value = 22.8;
    // memcpy(data.date_time, "2018-10-14T04:39:34", 20);

    // General struct 
    
    // Anemometro data
    memcpy(total_data.sensor_anemometro.sensor_id, "1234586564", 20);
    total_data.sensor_anemometro.vento_RPM = 15.95;
    total_data.sensor_anemometro.vento_KMH = 25.95;
    total_data.sensor_anemometro.vento_MS = 35.95;
    // Biruta data
    memcpy(total_data.sensor_biruta.sensor_id, "1234567891", 20);
    total_data.sensor_biruta.vento_direcao = 45.85;
    // Pluviometro data
    memcpy(total_data.sensor_pluviometro.sensor_id, "1112131415", 20);
    total_data.sensor_pluviometro.qtd_chuva = 85.7;
    // BME sensor 
    memcpy(total_data.sensor_bme.sensor_id, "16117181920", 20);
    total_data.sensor_bme.temp_celsius = 25.5;
    total_data.sensor_bme.humidity_percent = 85.7;
    total_data.sensor_bme.pressure_hPa = 98.8;

    memcpy(total_data.date_time, "2018-10-14T04:39:34", 20);

    // Building JSON message
    // sprintf(data_sent,"{\"sensor_id\": \"%s\", \"value\": %f, \"date_time\": \"%s\"}", 
    //                     data.sensor_id ,data.value,data.date_time);

    sprintf(data_sent,"{\"data_hora\": \"%s\", \"data\": {\"%s\": {\"vento_RPM\": %f, \"vento_KMH\": %f, \"vento_MS\": %f}, \"%s\": {\"vento_direcao\": %f}, \"%s\": {\"qtd_chuva\": %f}, \"%s\": {\"temp_celsius\": %f, \"humidity_percent\": %f, \"pressure_hPa\": %f }}}", 
                       total_data.date_time , total_data.sensor_anemometro.sensor_id, total_data.sensor_anemometro.vento_RPM, total_data.sensor_anemometro.vento_KMH, total_data.sensor_anemometro.vento_MS,
                       total_data.sensor_biruta.sensor_id, total_data.sensor_biruta.vento_direcao,
                       total_data.sensor_pluviometro.sensor_id, total_data.sensor_pluviometro.qtd_chuva,
                       total_data.sensor_bme.sensor_id, total_data.sensor_bme.temp_celsius, total_data.sensor_bme.humidity_percent, total_data.sensor_bme.pressure_hPa);
    
    // printf("%s", data_sent);
    // Define URL
    sprintf(request,"http://%s:%d/%s HTTP/1.1",host,portno,service_method);

    // Build Completed Request
    sprintf(completed_request,"POST %s\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %ld\r\n\r\n%s\r\n",
            request,host,strlen(data_sent),data_sent);

    // Create the socket 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    // Lookup the ip address
    server = gethostbyname(host);
    if (server == NULL) error("ERROR, no such host");

    // Fill in the structure server
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
        error("ERROR connecting");
    }else{
        printf("Success on socket connection");
    }

    total = strlen(completed_request);
    sent = 0;
    do {
        bytes = write(sockfd,completed_request+sent,total-sent);
        if (bytes < 0)
            error("ERROR writing message to socket");
        if (bytes == 0)
            break;
        sent+=bytes;
    } while (sent < total);
      
    memset(response,0,sizeof(response));
    total = sizeof(response)-1;
    received = 0;

    do {
        bytes = read(sockfd,response+received,total-received);
        if (bytes < 0)
            error("ERROR reading response from socket");
        if (bytes == 0)
            break;
        received+=bytes;
    } while (received < total);

    if (received == total)
        error("ERROR storing complete response from socket");

    // Close the socket
    close(sockfd);

    // Request response with header
    printf("Response:\n%s\n",response);

    
}