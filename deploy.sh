#!/bin/bash
set -e

# Set the directory to the project root
cd "$(dirname "$0")"
PROJECT_ROOT="$(pwd)"

echo "=== Building C++ backend ==="
cd "$PROJECT_ROOT/app/cpp_backend"
./build.sh

echo "=== Building frontend ==="
cd "$PROJECT_ROOT/app"
npm install
npm run build

echo "=== Updating reports ==="
cd "$PROJECT_ROOT"
./update-reports.sh

# Test OpenResty configuration
openresty -t

echo "=== Configuring systemd service ==="
systemctl daemon-reload
systemctl enable ledger-api.service
systemctl restart ledger-api.service
systemctl status ledger-api.service --no-pager

echo "=== Restarting OpenResty ==="
systemctl restart openresty
systemctl status openresty --no-pager

echo "=== Deployment completed successfully! ==="
echo "Your application is now available at https://ledger.abaj.ai (with HTTPS)" 