# VortexCoin ($VOR)

### Core Library --- Source-Code
This software represents **the main core of the VortexCoin** project, written in *C++* 






## Features
- **JSON support** for blockchain and wallet storing
- **SHA-256 algorithm** as standard hash system
- Integrated Transaction System
- Library Support for Core Applications



## API Documentation

The VortexCoin blockchain provides an **API** for interaction with external applications, such as wallet apps, stores, games, etc.

#### Transaction System

```http
  POST /transactions/{sender}/{receiver}/{amount}
```

| Parameters   | Variable Type       | Description                          |
| :---------- | :--------- | :---------------------------------- |
| `sender` | `string` | Transaction Sender |
| `receiver` |`string` | Transaction Receiver |
| `amount` | `double` | Amount |


#### Full Blockchain Return

```http
  GET /blockchain
```

#### Block Search from Block Hash

```http
  GET /block/{hash}
```

| Parameters   | Variable Type       | Description                          |
| :---------- | :--------- | :---------------------------------- |
| `hash` | `string` | Block Hash |


#### Wallet Balance

```http
  GET /balance/{wallet}
```

| Parameters   | Variable Type       | Description                          |
| :---------- | :--------- | :---------------------------------- |
| `wallet` | `string` | Wallet Address |




