import socket
import threading
import json
import time

# Configuration
HOST = '0.0.0.0'
PORT = 5555

# Global State
clients = {}       # {username: client_socket}
addresses = {}     # {client_socket: username}
groups = {}        # {group_name: [member_username1, member_username2, ...]}
group_admins = {}  # {group_name: admin_username}

def get_local_ip():
    """Helper to find the local IP address to show the user."""
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(('8.8.8.8', 80))
        IP = s.getsockname()[0]
    except Exception:
        IP = '127.0.0.1' 
    finally: 
        s.close() 
    return IP

def handle_client(client_socket):
    user_name = ""
    buffer = ""
    
    try:
        while True:
            # Increased buffer to 2MB to handle large file transfers reliably
            data = client_socket.recv(2 * 1024 * 1024) 
            if not data:
                break
            
            buffer += data.decode('utf-8')
            
            while "\n" in buffer:
                msg_str, buffer = buffer.split("\n", 1)
                if not msg_str.strip(): continue
                
                try:
                    message_data = json.loads(msg_str)
                except json.JSONDecodeError:
                    continue

                msg_type = message_data.get("type")
                
                # --- LOGIN ---
                if msg_type == "LOGIN":
                    new_name = message_data.get("name", "Unknown")
                    user_name = new_name
                    
                    if user_name in clients:
                        old_sock = clients[user_name]
                        try:
                            send_json(old_sock, {"type": "SERVER", "msg": "Logged in on another device. Disconnecting..."})
                            old_sock.close() 
                        except: pass 

                    clients[user_name] = client_socket
                    addresses[client_socket] = user_name
                    
                    print(f"[NEW CONNECTION] {user_name} connected.")
                    send_json(client_socket, {"type": "SERVER", "msg": f"Welcome, {user_name}!"})
                    broadcast_user_list()
                    send_group_list(user_name)

                elif not user_name: continue

                # --- MESSAGING ---
                elif msg_type == "PUBLIC_MSG":
                    broadcast_message(user_name, message_data["msg"])
                    
                elif msg_type == "PRIVATE_MSG":
                    target = message_data["target"]
                    content = message_data["msg"]
                    send_private_message(user_name, target, content)
                
                elif msg_type == "GROUP_MSG":
                    group_name = message_data["target"]
                    content = message_data["msg"]
                    send_group_message(user_name, group_name, content)

                elif msg_type == "VOICE_MSG":
                    target = message_data["target"]
                    is_group = message_data.get("is_group", False)
                    data_b64 = message_data["data"]
                    
                    if is_group:
                        send_group_file(user_name, target, "voice_note.wav", data_b64, is_voice=True)
                    elif target == "All":
                        # FIX: Added handler for Public Voice Messages
                        broadcast_file(user_name, "voice_note.wav", data_b64, is_voice=True)
                    else:
                        send_private_file(user_name, target, "voice_note.wav", data_b64, is_voice=True)
                
                # --- STREAMING RELAY ---
                elif msg_type in ["VIDEO_STREAM", "AUDIO_STREAM"]:
                    target = message_data["target"]
                    if target in clients:
                        send_json(clients[target], message_data)
     
                # --- FILE SHARING ---
                elif msg_type == "FILE":
                    target = message_data["target"]
                    filename = message_data["filename"]
                    file_data = message_data["data"]
                    is_group = message_data.get("is_group", False)

                    if is_group:
                        send_group_file(user_name, target, filename, file_data)
                    elif target == "All":
                        broadcast_file(user_name, filename, file_data)
                    else:
                        send_private_file(user_name, target, filename, file_data)

                # --- GROUP MANAGEMENT ---
                elif msg_type == "CREATE_GROUP":
                    group_name = message_data["group_name"]
                    if group_name in groups:
                        send_json(client_socket, {"type": "ERROR", "msg": "Group already exists."})
                    else:
                        groups[group_name] = [user_name]
                        group_admins[group_name] = user_name
                        send_group_list(user_name)
                        send_json(client_socket, {"type": "SERVER", "msg": f"Group '{group_name}' created."})

                elif msg_type == "ADD_MEMBER":
                    group_name = message_data["group_name"]
                    new_member = message_data["member_name"]
                    if group_name in groups:
                        if new_member in clients:
                            if new_member not in groups[group_name]:
                                groups[group_name].append(new_member)
                                send_group_list(new_member)
                                send_group_message("System", group_name, f"{user_name} added {new_member}")
                            else:
                                send_json(client_socket, {"type": "ERROR", "msg": "User already in group."})
                        else:
                            send_json(client_socket, {"type": "ERROR", "msg": "User not connected."})

                elif msg_type == "LEAVE_GROUP":
                    group_name = message_data["group_name"]
                    if group_name in groups and user_name in groups[group_name]:
                        groups[group_name].remove(user_name)
                        send_group_list(user_name)
                        send_group_message("System", group_name, f"{user_name} left the group.")
                        if not groups[group_name]:
                            del groups[group_name]
                            del group_admins[group_name]

                # --- CALLING FEATURES ---
                elif msg_type in ["VIDEO_CALL_REQUEST", "AUDIO_CALL_REQUEST"]:
                    target = message_data["target"]
                    if target in clients:
                        send_json(clients[target], {"type": msg_type, "from": user_name})
                    else:
                        send_json(client_socket, {"type": "CALL_FAILED", "msg": f"{target} is not online."})

                elif msg_type in ["CALL_ACCEPTED", "CALL_DECLINED", "CALL_ENDED"]:
                    target = message_data["target"]
                    if target in clients:
                        send_json(clients[target], {
                            "type": msg_type,
                            "from": user_name,
                            "call_type": message_data.get("call_type", "Video")
                        })

    except Exception as e:
        print(f"[ERROR] {user_name}: {e}")
    finally:
        cleanup_client(user_name, client_socket)

def cleanup_client(user_name, sock):
    if user_name and user_name in clients and clients[user_name] == sock:
        del clients[user_name]
    if sock in addresses:
        del addresses[sock]
    try: sock.close()
    except: pass
    if user_name not in clients:
        broadcast_user_list()
        print(f"[DISCONNECT] {user_name} disconnected.")

def send_json(sock, data):
    try:
        json_str = json.dumps(data) + "\n"
        sock.sendall(json_str.encode('utf-8'))
    except: pass

def broadcast_user_list():
    users = list(clients.keys())
    msg = {"type": "USER_LIST", "users": users}
    for sock in list(clients.values()): send_json(sock, msg)

def send_group_list(user_name):
    if user_name not in clients: return
    my_groups = []
    for g_name, members in groups.items():
        if user_name in members: my_groups.append(g_name)
    msg = {"type": "GROUP_LIST", "groups": my_groups}
    send_json(clients[user_name], msg)

def broadcast_message(sender, content):
    msg = {"type": "CHAT", "from": sender, "msg": content, "mode": "Public"}
    for user, sock in clients.items():
        if user != sender:
            send_json(sock, msg)

def send_private_message(sender, target, content):
    msg = {"type": "CHAT", "from": sender, "msg": content, "mode": "Private", "chat_id": sender}
    if target in clients:
        send_json(clients[target], msg)

def send_group_message(sender, group_name, content):
    if group_name not in groups: return
    msg = {"type": "CHAT", "from": sender, "msg": content, "mode": "Group", "chat_id": group_name}
    for member in groups[group_name]:
        if member in clients and member != sender:
            send_json(clients[member], msg)

# FIX: Added is_voice parameter to handle public voice notes
def broadcast_file(sender, filename, filedata, is_voice=False):
    msg_type = "VOICE_RX" if is_voice else "FILE_RX"
    msg = {"type": msg_type, "from": sender, "filename": filename, "data": filedata, "mode": "Public", "chat_id": "Public"}
    for user, sock in clients.items():
        if user != sender:
            send_json(sock, msg)

def send_private_file(sender, target, filename, filedata, is_voice=False):
    msg_type = "VOICE_RX" if is_voice else "FILE_RX"
    msg_to_target = {"type": msg_type, "from": sender, "filename": filename, "data": filedata, "mode": "Private", "chat_id": sender}
    if target in clients:
        send_json(clients[target], msg_to_target)

def send_group_file(sender, group_name, filename, filedata, is_voice=False):
    if group_name not in groups: return
    msg_type = "VOICE_RX" if is_voice else "FILE_RX"
    msg = {"type": msg_type, "from": sender, "filename": filename, "data": filedata, "mode": "Group", "chat_id": group_name}
    for member in groups[group_name]:
        if member in clients and member != sender:
            send_json(clients[member], msg)

def start_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((HOST, PORT))
    server.listen()
    local_ip = get_local_ip()
    print(f"--------------------------------------------------")
    print(f"[LISTENING] Server is running.")
    print(f"[CONNECT INFO] Users on other devices should connect to IP: {local_ip}")
    print(f"--------------------------------------------------")
    while True:
        conn, addr = server.accept()
        thread = threading.Thread(target=handle_client, args=(conn,))
        thread.start()
        print(f"[ACTIVE CONNECTIONS] {threading.active_count() - 1}")

if __name__ == "__main__":
    start_server()