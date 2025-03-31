# Ledger Web Application

A full-stack web application for managing and interacting with a [ledger-cli](https://ledger-cli.org/) file.

## Features

- View standard ledger reports (balance, register, budget, cleared)
- Add new transactions
- Edit existing transactions
- Delete transactions
- Responsive, modern UI built with Svelte.js

## Technology Stack

- **Frontend**: Svelte.js, Vite
- **Backend**: C++, Crow (lightweight web framework), nlohmann/json
- **Command-line**: Ledger CLI (for processing and querying the ledger file)

## Setup and Installation

### Prerequisites

- Node.js (v14 or higher) for the frontend
- C++17 compatible compiler (GCC 8+ or Clang 6+)
- CMake (v3.10+)
- Boost library (system, filesystem components)
- Ledger CLI installed and accessible via the command line

### Installation

1. Clone this repository:
```
git clone https://github.com/yourusername/ledger-web.git
cd ledger-web
```

2. Install frontend dependencies:
```
cd app
npm install
```

3. Set up the C++ backend:
```
cd cpp_backend
./get_deps.sh     # Download required libraries (Crow, nlohmann/json)
./build.sh        # Build the C++ backend
```

### Configuration

1. Configure the C++ backend API server:
   - Edit the configuration in `app/cpp_backend/ledger_api_config.json`.
   - Set the path to your ledger file, ledger command, and update-reports script.
   - Default port is 3001, which can be changed in the config file.

2. Configure the frontend:
   - The frontend will run on port 3003 by default.
   - The API proxy is configured to point to the backend server at port 3001.

### Running the Application

1. Start the C++ backend API server:
```
cd app/cpp_backend
./run.sh
```

2. Start the frontend development server:
```
cd app
npm run dev
```

3. Open your browser and navigate to:
```
http://localhost:3003
```

## Usage

- **Dashboard**: Overview of your accounts and recent transactions
- **Balance**: View account balances
- **Register**: View transaction history
- **Budget**: View budget reports
- **Cleared**: View cleared vs. pending transactions
- **Add Entry**: Add a new transaction
- **Edit**: Modify existing transactions

## Backend Architecture

The C++ backend provides a RESTful API that:
- Parses and modifies the ledger file directly
- Executes ledger-cli commands and processes the output
- Serves JSON data to the frontend
- Handles CRUD operations for transactions

The backend components include:
- **Config**: Singleton for managing configuration
- **LedgerManager**: Core class that interacts with the ledger file and processes commands
- **ApiHandler**: Handles HTTP requests and routes

## License

MIT

## Credits

Built using the ledger-cli and inspired by the need for a modern, web-based interface to interact with ledger files. 