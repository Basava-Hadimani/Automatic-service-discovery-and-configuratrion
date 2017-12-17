# Automatic-service-discovery-and-configuratrion
This project is used to connect clients to the required server automatically

Advertising all services provided by the server to the neighboring clients periodically and handling TCP connection between client and server once the service discovery is completed. It is always not possible to configure all the clients to the server specifically when the number of clients connecting to the server are more. Hence this project is designed to configure the clients to the required server. Client always listens to the advertisement of the server about its services. Once the client detects its server successfully, A TCP connection is maintained between then for further service.


Please find SERVER and CLIENT folder inside Home directory
Programs are writtin in C
