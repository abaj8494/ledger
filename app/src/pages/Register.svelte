<script>
  import { onMount } from 'svelte';
  import { push } from 'svelte-spa-router';
  
  let transactions = [];
  let allTransactions = []; // Store all transactions before filtering
  let isLoading = true;
  let errorMessage = '';
  let deleteConfirmIndex = null;
  let sortField = 'date';  // Default sort field
  let sortDirection = 'desc'; // Default sort direction (newest first)
  let searchTerm = '';
  
  // Generate a unique identifier for a transaction based on its properties
  function generateUniqueId(transaction) {
    // Combine date, payee and first posting details to create a unique signature
    const postingSignature = transaction.postings && transaction.postings.length > 0
      ? `${transaction.postings[0].account}:${transaction.postings[0].amount}`
      : '';
    return `${transaction.date}-${transaction.payee}-${postingSignature}`;
  }
  
  // Store the original unsorted transactions to maintain reference
  let originalTransactionsOrder = [];
  
  onMount(async () => {
    await loadTransactions();
  });
  
  async function loadTransactions() {
    isLoading = true;
    errorMessage = '';
    
    try {
      const response = await fetch('/api/transactions');
      
      if (response.ok) {
        const data = await response.json();
        console.log('Raw API response:', data); // Debug log
        
        // Store original data order - this is important for indexing later
        originalTransactionsOrder = [...data];
        
        // Process transactions to ensure they have proper display data
        allTransactions = data.map((transaction, index) => {
          // For main transaction row, use first posting's account and amount if available
          const firstPosting = transaction.postings && transaction.postings.length > 0 
            ? transaction.postings[0] : null;
          
          // Create the processed transaction object with the original index explicitly stored
          const processedTransaction = {
            ...transaction,
            // Store the server index explicitly - this is used for API calls
            serverIndex: index,
            account: firstPosting ? firstPosting.account || '' : '',
            amount: firstPosting ? firstPosting.amount || '' : '',
            balance: '', // Balance will be calculated in a future implementation
            // Generate a unique identifier for reliable lookups
            uniqueId: generateUniqueId(transaction),
            postings: transaction.postings ? transaction.postings.map(p => ({
              ...p,
              account: p.account || '',
              amount: p.amount || ''
            })) : []
          };
          
          if (transaction.id === undefined) {
            console.warn(`Transaction ${index} missing ID, using serverIndex`);
          }
          
          return processedTransaction;
        });
        
        // Sort transactions (already sorted by date from server, but apply any other sort)
        if (sortField !== 'date' || sortDirection !== 'desc') {
          sortTransactions(sortField, sortDirection);
        } else {
          transactions = [...allTransactions];
        }
      } else {
        errorMessage = 'Failed to load transaction data';
      }
    } catch (error) {
      console.error('Error fetching transaction data:', error);
      errorMessage = 'Error connecting to server';
    } finally {
      isLoading = false;
    }
  }
  
  function sortTransactions(field, direction) {
    sortField = field;
    sortDirection = direction;
    
    const transactionsToSort = searchTerm ? [...transactions] : [...allTransactions];
    
    transactions = transactionsToSort.sort((a, b) => {
      let valueA, valueB;
      
      // Handle different field types
      if (field === 'date') {
        // Convert dates to comparable format (remove slashes)
        valueA = a.date.replace(/\//g, '');
        valueB = b.date.replace(/\//g, '');
      } else if (field === 'amount') {
        // Extract numeric value from amount string
        valueA = parseFloat(a.amount.replace(/[^0-9.-]+/g, '')) || 0;
        valueB = parseFloat(b.amount.replace(/[^0-9.-]+/g, '')) || 0;
      } else {
        // For text fields (payee, account)
        valueA = (a[field] || '').toLowerCase();
        valueB = (b[field] || '').toLowerCase();
      }
      
      // Apply sort direction
      if (direction === 'asc') {
        return valueA > valueB ? 1 : valueA < valueB ? -1 : 0;
      } else {
        return valueA < valueB ? 1 : valueA > valueB ? -1 : 0;
      }
    });
  }
  
  function toggleSort(field) {
    if (sortField === field) {
      // Toggle direction if already sorting by this field
      sortTransactions(field, sortDirection === 'asc' ? 'desc' : 'asc');
    } else {
      // Default to descending for date, ascending for others
      const direction = field === 'date' ? 'desc' : 'asc';
      sortTransactions(field, direction);
    }
  }
  
  function searchTransactions(event) {
    searchTerm = event.target.value.toLowerCase().trim();
    
    if (!searchTerm) {
      // If search is cleared, show all transactions with current sort
      transactions = [...allTransactions];
      sortTransactions(sortField, sortDirection);
      return;
    }
    
    // Filter transactions based on search term
    transactions = allTransactions.filter(transaction => {
      return (
        transaction.date.toLowerCase().includes(searchTerm) ||
        transaction.payee.toLowerCase().includes(searchTerm) ||
        transaction.account.toLowerCase().includes(searchTerm) ||
        transaction.amount.toLowerCase().includes(searchTerm) ||
        // Also search in postings
        transaction.postings.some(posting => 
          posting.account.toLowerCase().includes(searchTerm) ||
          posting.amount.toLowerCase().includes(searchTerm) ||
          (posting.comment && posting.comment.toLowerCase().includes(searchTerm))
        )
      );
    });
    
    // Apply current sort to filtered results
    sortTransactions(sortField, sortDirection);
  }
  
  function showDeleteConfirm(index) {
    deleteConfirmIndex = index;
  }
  
  function cancelDelete() {
    deleteConfirmIndex = null;
  }
  
  async function confirmDelete(index) {
    const transaction = transactions[index];
    
    try {
      const response = await fetch(`/api/transactions/${transaction.id}`, {
        method: 'DELETE'
      });
      
      if (response.ok) {
        // Remove from both arrays
        allTransactions = allTransactions.filter(t => t.id !== transaction.id);
        transactions = transactions.filter((_, i) => i !== index);
        deleteConfirmIndex = null;
      } else {
        const error = await response.json();
        errorMessage = error.message || 'Failed to delete entry';
      }
    } catch (error) {
      console.error('Error deleting transaction:', error);
      errorMessage = 'Network error occurred';
    }
  }
  
  function editTransaction(index) {
    const transaction = transactions[index];
    console.log('Editing transaction:', transaction);
    console.log('Index in array:', index);
    console.log('Transaction uniqueId:', transaction.uniqueId);
    
    // Instead of using indices, use the transaction date and payee as identifiers
    // URL encode the parameters to handle special characters
    const encodedDate = encodeURIComponent(transaction.date);
    const encodedPayee = encodeURIComponent(transaction.payee);
    
    push(`/edit/${encodedDate}/${encodedPayee}`);
  }
</script>

<div class="register-page">
  <h1>Transaction Register</h1>
  
  {#if errorMessage}
    <div class="error-message">{errorMessage}</div>
  {/if}
  
  {#if isLoading}
    <div class="loading">Loading transactions...</div>
  {:else if transactions.length === 0 && !searchTerm}
    <div class="empty-state">
      <p>No transactions available</p>
      <button class="add-transaction-btn" on:click={() => push('/add')}>Add Your First Transaction</button>
    </div>
  {:else}
    <div class="search-bar">
      <input
        type="text"
        placeholder="Search transactions..."
        on:input={searchTransactions}
        value={searchTerm}
      />
    </div>
    
    <div class="sort-controls">
      <span>Sort by:</span>
      <button 
        class={sortField === 'date' ? 'sort-btn active' : 'sort-btn'} 
        on:click={() => toggleSort('date')}
      >
        Date {sortField === 'date' ? (sortDirection === 'asc' ? '↑' : '↓') : ''}
      </button>
      <button 
        class={sortField === 'payee' ? 'sort-btn active' : 'sort-btn'} 
        on:click={() => toggleSort('payee')}
      >
        Payee {sortField === 'payee' ? (sortDirection === 'asc' ? '↑' : '↓') : ''}
      </button>
      <button 
        class={sortField === 'account' ? 'sort-btn active' : 'sort-btn'} 
        on:click={() => toggleSort('account')}
      >
        Account {sortField === 'account' ? (sortDirection === 'asc' ? '↑' : '↓') : ''}
      </button>
      <button 
        class={sortField === 'amount' ? 'sort-btn active' : 'sort-btn'} 
        on:click={() => toggleSort('amount')}
      >
        Amount {sortField === 'amount' ? (sortDirection === 'asc' ? '↑' : '↓') : ''}
      </button>
    </div>
    
    {#if transactions.length === 0 && searchTerm}
      <div class="empty-search">
        <p>No transactions found for "{searchTerm}"</p>
        <button class="clear-search-btn" on:click={() => { searchTerm = ''; searchTransactions({ target: { value: '' } }); }}>
          Clear Search
        </button>
      </div>
    {:else}
      <div class="transactions-container">
        <table class="transactions-table">
          <thead>
            <tr>
              <th class="date-column">Date</th>
              <th class="payee-column">Payee</th>
              <th class="account-column">Account</th>
              <th class="amount-column">Amount</th>
              <th class="balance-column">Balance</th>
              <th class="actions-column">Actions</th>
            </tr>
          </thead>
          <tbody>
            {#each transactions as transaction, index}
              <tr class={transaction.pending ? 'transaction-row pending' : 'transaction-row'}>
                <td class="date-cell">
                  {transaction.date}
                </td>
                <td class="payee-cell">
                  {transaction.payee}
                </td>
                <td class="account-cell">
                  {transaction.account}
                </td>
                <td class="amount-cell">
                  {transaction.amount}
                </td>
                <td class="balance-cell">
                  {transaction.balance}
                </td>
                <td class="actions-cell">
                  {#if deleteConfirmIndex === index}
                    <div class="delete-confirm">
                      <span>Delete?</span>
                      <button class="confirm-btn" on:click={() => confirmDelete(index)}>Yes</button>
                      <button class="cancel-btn" on:click={cancelDelete}>No</button>
                    </div>
                  {:else}
                    <button class="edit-btn" on:click={() => editTransaction(index)}>
                      Edit
                    </button>
                    <button class="delete-btn" on:click={() => showDeleteConfirm(index)}>
                      Delete
                    </button>
                  {/if}
                </td>
              </tr>
              {#if transaction.postings && transaction.postings.length > 0}
                {#each transaction.postings as posting}
                  <tr class="posting-row">
                    <td></td>
                    <td>
                      {#if posting.payee}
                        <span class="posting-payee">{posting.payee}</span>
                      {/if}
                    </td>
                    <td class="posting-account">{posting.account}</td>
                    <td class="posting-amount">{posting.amount}</td>
                    <td></td>
                    <td></td>
                  </tr>
                {/each}
              {/if}
            {/each}
          </tbody>
        </table>
      </div>
    {/if}
  {/if}
</div>

<style>
  .register-page {
    width: 100%;
  }
  
  h1 {
    margin-bottom: 20px;
  }
  
  .loading, .empty-state, .empty-search {
    margin: 40px 0;
    text-align: center;
    font-style: italic;
    color: #777;
  }
  
  .empty-state button, .empty-search button {
    display: block;
    margin: 20px auto;
    padding: 10px 20px;
    background: #3498db;
    color: white;
    border: none;
    border-radius: 4px;
    cursor: pointer;
    font-weight: bold;
  }
  
  .search-bar {
    margin-bottom: 15px;
  }
  
  .search-bar input {
    width: 100%;
    padding: 10px 15px;
    border: 1px solid #ddd;
    border-radius: 4px;
    font-size: 1rem;
    box-shadow: 0 1px 3px rgba(0, 0, 0, 0.05);
  }
  
  .search-bar input:focus {
    outline: none;
    border-color: #3498db;
    box-shadow: 0 0 0 2px rgba(52, 152, 219, 0.2);
  }
  
  .clear-search-btn {
    background: #95a5a6;
    color: white;
  }
  
  .error-message {
    background: #ffecec;
    color: #e74c3c;
    padding: 15px;
    border-radius: 4px;
    margin: 20px 0;
    border-left: 4px solid #e74c3c;
  }
  
  .sort-controls {
    margin-bottom: 15px;
    display: flex;
    align-items: center;
    flex-wrap: wrap;
    gap: 8px;
  }
  
  .sort-controls span {
    font-weight: bold;
    margin-right: 5px;
  }
  
  .sort-btn {
    padding: 8px 12px;
    background: #f5f5f5;
    border: 1px solid #ddd;
    border-radius: 4px;
    cursor: pointer;
    font-size: 0.9rem;
    transition: all 0.2s;
  }
  
  .sort-btn:hover {
    background: #e9e9e9;
  }
  
  .sort-btn.active {
    background: #3498db;
    color: white;
    border-color: #2980b9;
  }
  
  .transactions-container {
    background: white;
    border-radius: 8px;
    box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
    overflow: hidden;
  }
  
  .transactions-table {
    width: 100%;
    border-collapse: collapse;
  }
  
  .transactions-table th {
    background: #f5f5f5;
    padding: 12px 15px;
    text-align: left;
    border-bottom: 2px solid #ddd;
    font-weight: bold;
  }
  
  .transactions-table td {
    padding: 10px 15px;
    border-bottom: 1px solid #eee;
  }
  
  .date-column {
    width: 100px;
  }
  
  .amount-column, .balance-column {
    width: 120px;
    text-align: right;
  }
  
  .actions-column {
    width: 160px;
    text-align: center;
  }
  
  .amount-cell, .balance-cell {
    font-family: monospace;
    text-align: right;
    white-space: nowrap;
  }
  
  .posting-row {
    background-color: #f9f9f9;
    font-size: 0.9em;
  }
  
  .posting-account {
    padding-left: 20px;
    color: #555;
  }
  
  .posting-amount {
    font-family: monospace;
    text-align: right;
  }
  
  .pending {
    background-color: #fff8e1;
  }
  
  .actions-cell {
    white-space: nowrap;
    text-align: center;
  }
  
  .edit-btn, .delete-btn, .confirm-btn, .cancel-btn {
    padding: 5px 10px;
    border: none;
    border-radius: 4px;
    cursor: pointer;
    font-size: 0.8rem;
    margin: 0 2px;
  }
  
  .edit-btn {
    background-color: #3498db;
    color: white;
  }
  
  .delete-btn {
    background-color: #e74c3c;
    color: white;
  }
  
  .confirm-btn {
    background-color: #e74c3c;
    color: white;
  }
  
  .cancel-btn {
    background-color: #95a5a6;
    color: white;
  }
  
  .delete-confirm {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 5px;
  }
  
  .delete-confirm span {
    font-size: 0.8rem;
    font-weight: bold;
  }
</style> 