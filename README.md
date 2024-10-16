# README: Instant Messaging Application with Chat Rooms and File Sharing 🚀💬📁

## Overview
This project is an **Instant Messaging Application** designed to offer real-time communication with chat rooms and file-sharing capabilities through a central server. It is inspired by platforms like Slack, Discord, and IRC, and aims to deliver essential features such as chat room participation, real-time messaging, and file sharing. This project combines crucial aspects of **network programming** and **user management** to create a real-time communication service. 

### Core Objectives:
1. Allow multiple users to connect to a server, participate in chat rooms, and exchange files in real-time.
2. Manage user authentication, access rights, and the security of communications 🔐.
3. Provide an interactive real-time experience with file-sharing and management features 📂.
4. **Additional feature**: Multi-server synchronization to ensure seamless communication and data sharing across multiple servers 🌐.

## Features 🎯
### 1. **User Authentication & Access Management** 🔐
   - Users must log in to access the service.
   - User registration includes username, age, gender, and password.
   - Secure handling of user credentials and sessions.

### 2. **Chat Rooms (Discussion Salons)** 💬
   - Users can join or create thematic chat rooms where real-time communication takes place.
   - Support for multiple users within a single chat room.

### 3. **Real-Time Messaging** 📨
   - Instant messaging functionality allows users to send and receive messages in real-time within the chat rooms.
   - Users can send direct messages to other members in the same chat room.

### 4. **File Sharing** 📁
   - Users can upload files to a shared space in the chat room, allowing others to download them.
   - Users can also list available files within the chat room for easy access.
   - Access rights are managed to ensure only authorized users can access or share files.

### 5. **Multi-Server Synchronization (Extension)** 🌐
   - The application supports multi-server synchronization, ensuring that chat rooms, messages, and files are updated across multiple servers. This enhances scalability and reliability by distributing the workload.

## Technical Requirements ⚙️

### Core Requirements:
- **User Management and Authentication**: Manage users with login, registration, and access control 🔑.
- **Chat Rooms**: Support for multiple chat rooms where users can send and receive messages in real-time 💬.
- **Real-Time Messaging**: Provide low-latency messaging for a real-time communication experience 🚀.
- **File Sharing**: Enable users to upload, download, and list files in the chat rooms 📂.
- **Access Rights Management**: Ensure secure access to shared files and resources 🔒.

### Extended Features (Future Enhancements) ✨:
- **Message Archiving**: Save chat histories for later retrieval.
- **Multi-Server Synchronization** (Implemented) 🌍: Synchronize messages and files across multiple servers.
- **File Versioning**: Handle file version collisions when multiple versions of a file are uploaded.
- **Secure Transfers (SSL/TLS)**: Secure communications with encryption protocols like SSL/TLS 🔐.
- **Flow Control Protocols**: Improve data transfer performance by managing data flow between servers and clients ⚡.

## Installation and Compilation 🛠️

To install and compile the project, follow these steps:

1. **Clone the repository**:
   ```bash
   git clone <repository_url>
   cd <project_directory>
   ```

2. **Compile the code**:
   Use the `make` command to compile the project:
   ```bash
   make all
   ```

3. **Run the program**:
   Execute the compiled binary to start the server:
   ```bash
   ./<executable_name>
   ```

## User Interaction Guide 📝

Once the system is running, users can interact with the service using the following commands.

### 1. Connection Commands 🌐
Upon connecting to the system, the following commands are available:

```text
--------------------
Available commands:
login <username> <password>
create_user <username> <age> <Gender (M/F)> <password>

commands for logged-in users:
list_groups
join_group <group_name>
--------------------
Enter command:
```

#### Command Descriptions 📖:
- `login <username> <password>`: Logs the user in with their credentials.
- `create_user <username> <age> <Gender (M/F)> <password>`: Creates a new user with the specified details.
- `list_groups`: Displays a list of available chat rooms.
- `join_group <group_name>`: Joins the user to the specified chat room.

### 2. Group Commands 👥
Once a user joins a group, additional commands are enabled:

```text
--------------------
Available commands:
<message> to send a message
upload_file <file path> to upload a file to the group
download_file <file name> to download a file from the group
list_files to list available files in the group
--------------------
Enter command:
```

#### Group-Specific Commands 🛠️:
- Send a message: Type the message and press enter to send it to the chat room.
- `upload_file <file path>`: Upload a file to the group's shared space.
- `download_file <file name>`: Download a file from the group's shared files.
- `list_files`: List all available files in the chat room.

### Exiting the Application 🛑
To exit, you can use the `Ctrl + C` command or follow the appropriate exit commands if specified.

## Troubleshooting 🐛

- **Connection issues**: Ensure the server is running, and the correct port is being used for connection.
- **Login problems**: Verify the credentials used during login or ensure the user is registered.
- **File access issues**: Confirm that the user has the appropriate permissions to access the files.
