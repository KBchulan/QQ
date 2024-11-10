CREATE TABLE users (
    user_id BIGINT PRIMARY KEY,
    username VARCHAR(50) NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    avatar_url VARCHAR(255),
    status TINYINT DEFAULT 0
);

CREATE TABLE messages (
    msg_id BIGINT PRIMARY KEY,
    sender_id BIGINT,
    receiver_id BIGINT,
    content TEXT,
    msg_type TINYINT,
    send_time TIMESTAMP,
    FOREIGN KEY (sender_id) REFERENCES users(user_id),
    FOREIGN KEY (receiver_id) REFERENCES users(user_id)
); 