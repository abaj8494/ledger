#!/bin/bash

# Path to the ledger2html script
LEDGER2HTML="/var/www/ledger/src/ledger2html"

# Path to the ledger file
LEDGER_FILE="/var/www/ledger/data/main.ledger"

# Directory for reports
REPORTS_DIR="/var/www/ledger/app/dist/reports"

# Create reports directory if it doesn't exist
mkdir -p "$REPORTS_DIR"

# Generate balance report
echo "Generating balance report..."
"$LEDGER2HTML" -f "$LEDGER_FILE" balance > "$REPORTS_DIR/balance.html"

# Generate register report
echo "Generating register report..."
"$LEDGER2HTML" -f "$LEDGER_FILE" register > "$REPORTS_DIR/register.html"

# Generate cleared report
echo "Generating cleared report..."
"$LEDGER2HTML" -f "$LEDGER_FILE" cleared > "$REPORTS_DIR/cleared.html"

# Generate budget report
echo "Generating budget report..."
"$LEDGER2HTML" -f "$LEDGER_FILE" budget > "$REPORTS_DIR/budget.html"

echo "Reports generated successfully!" 