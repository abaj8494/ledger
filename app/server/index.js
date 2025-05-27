const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3001;
const LEDGER_FILE = process.env.LEDGER_FILE || '/var/www/ledger/data/demo.ledger';
const LEDGER_CMD = process.env.LEDGER_CMD || 'ledger';

// Middleware
app.use(cors());
app.use(bodyParser.json());

// Helper function to run ledger commands
function runLedgerCommand(args) {
  try {
    const command = `${LEDGER_CMD} -f ${LEDGER_FILE} ${args}`;
    const output = execSync(command, { encoding: 'utf-8' });
    return output;
  } catch (error) {
    console.error(`Error executing ledger command: ${error.message}`);
    throw new Error(`Failed to execute ledger command: ${error.message}`);
  }
}

// Parse ledger file content
function parseLedgerFile() {
  try {
    const content = fs.readFileSync(LEDGER_FILE, 'utf-8');
    return content;
  } catch (error) {
    console.error(`Error reading ledger file: ${error.message}`);
    throw new Error(`Failed to read ledger file: ${error.message}`);
  }
}

// Write to ledger file
function writeLedgerFile(content) {
  try {
    // Create a backup first
    const backupPath = `${LEDGER_FILE}.bak`;
    fs.copyFileSync(LEDGER_FILE, backupPath);
    
    // Write the new content
    fs.writeFileSync(LEDGER_FILE, content, 'utf-8');
    
    return true;
  } catch (error) {
    console.error(`Error writing to ledger file: ${error.message}`);
    throw new Error(`Failed to write to ledger file: ${error.message}`);
  }
}

// Parse a transaction from ledger file content
function parseTransaction(content, index) {
  const transactions = [];
  let currentTransaction = null;
  let currentPosting = null;
  
  const lines = content.split('\n');
  
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i].trim();
    
    // Skip empty lines
    if (!line) continue;
    
    // Check if this is a transaction line (starts with a date)
    const transactionMatch = line.match(/^(\d{4}\/\d{2}\/\d{2})\s+(\*|\!)?\s*(.*)/);
    
    if (transactionMatch) {
      // If we already have a transaction, push it to the array
      if (currentTransaction) {
        transactions.push(currentTransaction);
      }
      
      // Start a new transaction
      currentTransaction = {
        startLine: i,
        date: transactionMatch[1],
        cleared: transactionMatch[2] === '*',
        pending: transactionMatch[2] === '!',
        payee: transactionMatch[3],
        postings: []
      };
    } else if (currentTransaction) {
      // This is a posting or a comment for the current transaction
      const postingMatch = line.match(/^\s+([^;]+?)(?:\s{2,}|\t)([$€£]?\-?\d[\d,\.]*)/);
      const commentMatch = line.match(/^\s+;(.*)/);
      
      if (postingMatch) {
        currentPosting = {
          account: postingMatch[1].trim(),
          amount: postingMatch[2].trim(),
          comment: ''
        };
        currentTransaction.postings.push(currentPosting);
      } else if (commentMatch && currentPosting) {
        currentPosting.comment = commentMatch[1].trim();
      } else if (line.match(/^\s+[^\s]/)) {
        // This is a posting without an amount
        const accountMatch = line.match(/^\s+([^;]+)/);
        
        if (accountMatch) {
          currentPosting = {
            account: accountMatch[1].trim(),
            amount: '',
            comment: ''
          };
          currentTransaction.postings.push(currentPosting);
        }
      }
    }
  }
  
  // Don't forget to add the last transaction
  if (currentTransaction) {
    transactions.push(currentTransaction);
  }

  console.log(`Parsed ${transactions.length} transactions from ledger file`);

  // If requesting all transactions, add id field to each transaction
  if (index === undefined) {
    // Add IDs to all transactions
    const result = transactions.map((transaction, idx) => {
      transaction.id = idx;  // Add ID directly to transaction object
      return transaction;
    });
    
    // Debug log
    console.log(`Returning ${result.length} transactions with IDs`);
    
    return result;
  }
  
  // Return the requested transaction or null if index is out of bounds
  const transaction = transactions[index] || null;
  if (transaction) {
    transaction.id = index;  // Add ID to individual transaction
    console.log(`Returning single transaction with ID ${index}: ${transaction.payee}`);
  } else {
    console.log(`Transaction with index ${index} not found in ${transactions.length} transactions`);
  }
  
  return transaction;
}

// Format a transaction for insertion into the ledger file
function formatTransaction(transaction) {
  let content = `${transaction.date} ${transaction.isCleared ? '*' : ''} ${transaction.payee}\n`;
  
  transaction.postings.forEach(posting => {
    content += `  ${posting.account}`;
    
    if (posting.amount) {
      // Ensure there are at least two spaces between account and amount
      const amountPadding = Math.max(2, 50 - posting.account.length);
      content += ' '.repeat(amountPadding) + posting.amount;
    }
    
    if (posting.comment) {
      content += `  ; ${posting.comment}`;
    }
    
    content += '\n';
  });
  
  return content;
}

// Helper to serialize account data
function serializeAccountData(ledgerOutput) {
  try {
    const lines = ledgerOutput.trim().split('\n');
    const result = [];
    
    lines.forEach(line => {
      if (!line.trim()) return;
      
      const spacePadding = line.search(/\S/);
      const level = Math.max(0, Math.floor(spacePadding / 2));
      
      const parts = line.trim().split(/\s{2,}/);
      if (parts.length >= 2) {
        result.push({
          amount: parts[0],
          account: parts[1],
          level
        });
      }
    });
    
    return result;
  } catch (error) {
    console.error(`Error serializing account data: ${error.message}`);
    return [];
  }
}

// Routes
// Get account summary for dashboard
app.get('/api/summary', (req, res) => {
  try {
    const output = runLedgerCommand('balance ^Assets ^Liabilities --depth 2');
    const accounts = serializeAccountData(output);
    res.json(accounts);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// Get recent transactions
app.get('/api/transactions', (req, res) => {
  try {
    const limit = req.query.limit ? parseInt(req.query.limit) : null;
    const content = parseLedgerFile();
    let transactions = parseTransaction(content);
    
    // Add explicit index-based IDs before reversing
    const totalCount = transactions.length;
    transactions = transactions.map((transaction, idx) => {
      // Use a stable identifier that won't change with reversal
      transaction.id = totalCount - 1 - idx;
      transaction.originalIndex = idx; // For debugging
      return transaction;
    });
    
    console.log(`Added IDs to ${transactions.length} transactions`);
    
    // Most recent first
    transactions.reverse();
    
    if (limit) {
      transactions = transactions.slice(0, limit);
    }
    
    // Verify some IDs in the response
    if (transactions.length > 0) {
      console.log(`First transaction: ${transactions[0].payee}, id=${transactions[0].id}, originalIndex=${transactions[0].originalIndex}`);
      if (transactions.length > 1) {
        console.log(`Second transaction: ${transactions[1].payee}, id=${transactions[1].id}, originalIndex=${transactions[1].originalIndex}`);
      }
    }
    
    res.json(transactions);
  } catch (error) {
    console.error(`Error getting transactions: ${error.message}`);
    res.status(500).json({ error: error.message });
  }
});

// Get all accounts for autocomplete
app.get('/api/accounts', (req, res) => {
  try {
    const output = runLedgerCommand('accounts');
    const accounts = output.trim().split('\n');
    res.json(accounts);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// Get balance report
app.get('/api/balance', (req, res) => {
  try {
    const output = runLedgerCommand('balance');
    const balanceData = serializeAccountData(output);
    res.json(balanceData);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// Get register report
app.get('/api/register', (req, res) => {
  try {
    const output = runLedgerCommand('register');
    // This is a simplified approach - in a real app, you'd want to parse more carefully
    const lines = output.trim().split('\n');
    const registerData = [];
    
    lines.forEach(line => {
      const parts = line.trim().split(/\s{2,}/);
      if (parts.length >= 5) {
        registerData.push({
          date: parts[0],
          payee: parts[1],
          account: parts[2],
          amount: parts[3],
          balance: parts[4]
        });
      }
    });
    
    res.json(registerData);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// Get budget report
app.get('/api/budget', (req, res) => {
  try {
    // Using tag to identify budget items
    const output = runLedgerCommand('balance ^Expenses --budget');
    const budgetData = [];
    
    const lines = output.trim().split('\n');
    lines.forEach(line => {
      if (!line.trim()) return;
      
      const spacePadding = line.search(/\S/);
      const level = Math.max(0, Math.floor(spacePadding / 2));
      
      const parts = line.trim().split(/\s{2,}/);
      if (parts.length >= 4) {
        budgetData.push({
          actual: parts[0],
          budget: parts[1],
          remaining: parts[2],
          percent: parts[3],
          account: parts[4],
          level
        });
      }
    });
    
    res.json(budgetData);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// Get cleared report
app.get('/api/cleared', (req, res) => {
  try {
    const output = runLedgerCommand('balance --cleared --pending');
    const clearedData = [];
    
    const lines = output.trim().split('\n');
    lines.forEach(line => {
      if (!line.trim()) return;
      
      const spacePadding = line.search(/\S/);
      const level = Math.max(0, Math.floor(spacePadding / 2));
      
      const parts = line.trim().split(/\s{2,}/);
      if (parts.length >= 3) {
        clearedData.push({
          cleared: parts[0],
          pending: parts[1],
          lastCleared: parts[2] || '',
          account: parts[3],
          level
        });
      }
    });
    
    res.json(clearedData);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// Get a single transaction
app.get('/api/transactions/:index', (req, res) => {
  try {
    const index = parseInt(req.params.index);
    
    console.log(`Fetching transaction with index: ${index}`);
    
    if (isNaN(index)) {
      console.log('Invalid index format');
      return res.status(400).json({ error: 'Invalid transaction index' });
    }
    
    const content = parseLedgerFile();
    const allTransactions = parseTransaction(content);
    console.log(`Total transactions in file: ${allTransactions.length}`);
    
    // Convert UI index to file index - the IDs are reversed
    const fileIndex = allTransactions.length - 1 - index;
    console.log(`Converting UI index ${index} to file index ${fileIndex}`);
    
    if (fileIndex < 0 || fileIndex >= allTransactions.length) {
      console.log(`File index ${fileIndex} is out of bounds (max: ${allTransactions.length - 1})`);
      return res.status(404).json({ error: 'Transaction not found - index out of bounds' });
    }
    
    const transaction = parseTransaction(content, fileIndex);
    
    if (!transaction) {
      console.log(`Transaction with file index ${fileIndex} not found`);
      return res.status(404).json({ error: 'Transaction not found' });
    }
    
    // Add the correct ID to match what the UI expects
    transaction.id = index;
    
    console.log(`Found transaction: ${transaction.date} ${transaction.payee}`);
    
    res.json(transaction);
  } catch (error) {
    console.error(`Error fetching transaction: ${error.message}`);
    res.status(500).json({ error: error.message });
  }
});

// Add a new transaction
app.post('/api/transactions', (req, res) => {
  try {
    const { date, payee, isCleared, postings } = req.body;
    
    if (!date || !payee || !postings || !Array.isArray(postings) || postings.length < 1) {
      return res.status(400).json({ error: 'Invalid transaction data' });
    }
    
    const newTransaction = {
      date,
      payee,
      isCleared,
      postings
    };
    
    const formattedTransaction = formatTransaction(newTransaction);
    const content = parseLedgerFile();
    
    // Append the new transaction to the file
    const newContent = content + '\n' + formattedTransaction;
    writeLedgerFile(newContent);
    
    // Update the reports
    const updateCmd = 'bash /var/www/ledger/update-reports.sh';
    execSync(updateCmd);
    
    res.status(201).json({ message: 'Transaction added successfully' });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// Update a transaction
app.put('/api/transactions/:index', (req, res) => {
  try {
    const index = parseInt(req.params.index);
    
    console.log(`Updating transaction with index: ${index}`);
    
    if (isNaN(index)) {
      return res.status(400).json({ error: 'Invalid transaction index' });
    }
    
    const { date, payee, isCleared, postings } = req.body;
    
    if (!date || !payee || !postings || !Array.isArray(postings) || postings.length < 1) {
      return res.status(400).json({ error: 'Invalid transaction data' });
    }
    
    const content = parseLedgerFile();
    const transactions = parseTransaction(content);
    
    // Convert UI index to file index - the IDs are reversed
    const fileIndex = transactions.length - 1 - index;
    console.log(`Converting UI index ${index} to file index ${fileIndex}`);
    
    if (fileIndex < 0 || fileIndex >= transactions.length) {
      console.log(`File index ${fileIndex} is out of bounds (max: ${transactions.length - 1})`);
      return res.status(404).json({ error: 'Transaction not found - index out of bounds' });
    }
    
    console.log(`Updating transaction #${fileIndex}: ${transactions[fileIndex].payee}`);
    
    // Replace the transaction
    const updatedTransaction = {
      date,
      payee,
      isCleared,
      postings
    };
    
    const formattedTransaction = formatTransaction(updatedTransaction);
    
    // Find the start and end of the transaction to replace
    const startLine = transactions[fileIndex].startLine;
    let endLine = fileIndex < transactions.length - 1 ? transactions[fileIndex + 1].startLine - 1 : null;
    
    const lines = content.split('\n');
    let newContent;
    
    if (endLine === null) {
      // This is the last transaction
      endLine = lines.length - 1;
      
      // Remove the old transaction
      newContent = lines.slice(0, startLine).join('\n');
      
      // Append the new transaction
      newContent += '\n' + formattedTransaction;
    } else {
      // Remove the old transaction and insert the new one
      newContent = lines.slice(0, startLine).join('\n') + 
        '\n' + formattedTransaction + 
        lines.slice(endLine + 1).join('\n');
    }
    
    writeLedgerFile(newContent);
    
    // Update the reports
    const updateCmd = 'bash /var/www/ledger/update-reports.sh';
    execSync(updateCmd);
    
    res.json({ message: 'Transaction updated successfully' });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// Delete a transaction
app.delete('/api/transactions/:index', (req, res) => {
  try {
    const index = parseInt(req.params.index);
    
    console.log(`Deleting transaction with index: ${index}`);
    
    if (isNaN(index)) {
      return res.status(400).json({ error: 'Invalid transaction index' });
    }
    
    const content = parseLedgerFile();
    const transactions = parseTransaction(content);
    
    // Convert UI index to file index - the IDs are reversed
    const fileIndex = transactions.length - 1 - index;
    console.log(`Converting UI index ${index} to file index ${fileIndex}`);
    
    if (fileIndex < 0 || fileIndex >= transactions.length) {
      console.log(`File index ${fileIndex} is out of bounds (max: ${transactions.length - 1})`);
      return res.status(404).json({ error: 'Transaction not found - index out of bounds' });
    }
    
    console.log(`Deleting transaction #${fileIndex}: ${transactions[fileIndex].payee}`);
    
    // Find the start and end of the transaction to delete
    const startLine = transactions[fileIndex].startLine;
    let endLine = fileIndex < transactions.length - 1 ? transactions[fileIndex + 1].startLine - 1 : null;
    
    const lines = content.split('\n');
    let newContent;
    
    if (endLine === null) {
      // This is the last transaction
      newContent = lines.slice(0, startLine).join('\n');
    } else {
      // Remove just this transaction
      newContent = lines.slice(0, startLine).join('\n') + 
        lines.slice(endLine + 1).join('\n');
    }
    
    writeLedgerFile(newContent);
    
    // Update the reports
    const updateCmd = 'bash /var/www/ledger/update-reports.sh';
    execSync(updateCmd);
    
    res.json({ message: 'Transaction deleted successfully' });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// Start the server
app.listen(PORT, () => {
  console.log(`Ledger API server running on port ${PORT}`);
}); 