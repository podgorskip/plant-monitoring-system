DROP TABLE IF EXISTS user_details;

CREATE TABLE user_details (
    id BIGINT PRIMARY KEY,
    mac CHAR(18) NOT NULL UNIQUE,
    first_name VARCHAR(255) NOT NULL,
    last_name VARCHAR(255) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    phone_number CHAR(9) NOT NULL UNIQUE,
    password TEXT NOT NULL,
    role VARCHAR(20) NOT NULL,
    verified BOOLEAN NOT NULL DEFAULT false
);

DROP TABLE IF EXISTS device;

CREATE TABLE device (
    id BIGINT PRIMARY KEY,
    mac CHAR(18) NOT NULL UNIQUE,
    user_id BIGINT REFERENCES user_details(id),
    air_humidity_lower_threshold numeric(5, 2),
    air_humidity_upper_threshold numeric(5, 2),
    soil_humidity_lower_threshold numeric(5, 2),
    soil_humidity_upper_threshold numeric(5, 2),
    temperature_lower_threshold numeric(5, 2),
    temperature_upper_threshold numeric(5, 2),
    insolation_lower_threshold numeric(5, 2),
    insolation_upper_threshold numeric(5, 2),
    creation_date TIMESTAMP NOT NULL DEFAULT NOW()
);

DROP TABLE IF EXISTS air_humidity;

CREATE TABLE air_humidity (
    id BIGINT PRIMARY KEY,
    value NUMERIC(5, 2) NOT NULL,
    device_id BIGINT NOT NULL REFERENCES device(id),
    date TIMESTAMP NOT NULL DEFAULT NOW()
);

DROP TABLE IF EXISTS soil_humidity;

CREATE TABLE soil_humidity (
    id BIGINT PRIMARY KEY,
    value NUMERIC(5, 2) NOT NULL,
    device_id BIGINT NOT NULL REFERENCES device(id),
    date TIMESTAMP NOT NULL DEFAULT NOW()
);

DROP TABLE IF EXISTS temperature;

CREATE TABLE temperature (
    id BIGINT PRIMARY KEY,
    value NUMERIC(5, 2) NOT NULL,
    device_id BIGINT NOT NULL REFERENCES device(id),
    date TIMESTAMP NOT NULL DEFAULT NOW()
);

DROP TABLE IF EXISTS insolation;

CREATE TABLE insolation (
    id BIGINT PRIMARY KEY,
    value NUMERIC(5, 2) NOT NULL,
    device_id BIGINT NOT NULL REFERENCES device(id),
    date TIMESTAMP NOT NULL DEFAULT NOW()
);

DROP TABLE IF EXISTS notification;

CREATE TABLE notification (
    id BIGINT PRIMARY KEY,
    device_id BIGINT NOT NULL REFERENCES device(id),
    type VARCHAR(32) NOT NULL,
    min BOOLEAN,
    date TIMESTAMP NOT NULL DEFAULT now()
);

DROP TABLE IF EXISTS water_request;

CREATE TABLE water_request (
    id BIGINT PRIMARY KEY,
    device_id BIGINT NOT NULL REFERENCES device(id),
    time INT NOT NULL,
    date TIMESTAMP NOT NULL DEFAULT now()
);
