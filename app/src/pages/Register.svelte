<script>
  import { onMount } from 'svelte';
  import { push } from 'svelte-spa-router';
  
  let transactions = [];
  let isLoading = true;
  let errorMessage = '';
  let deleteConfirmIndex = null;
  
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
        
        // Process transactions to ensure they have proper display data
        transactions = data.map(transaction => {
          // For main transaction row, use first posting's account and amount if available
          const firstPosting = transaction.postings && transaction.postings.length > 0 
            ? transaction.postings[0] : null;
          
          // Debug logging to check what we're getting
          if (firstPosting) {
            console.log('First posting:', firstPosting);
          }
          
          return {
            ...transaction,
            account: firstPosting ? firstPosting.account || '' : '',
            amount: firstPosting ? firstPosting.amount || '' : '',
            balance: '', // Balance will be calculated in a future implementation
            postings: transaction.postings ? transaction.postings.map(p => ({
              ...p,
              account: p.account || '',
              amount: p.amount || ''
            })) : []
          };
        });
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
  
  function showDeleteConfirm(index) {
    deleteConfirmIndex = index;
  }
  
  function cancelDelete() {
    deleteConfirmIndex = null;
  }
  
  async function confirmDelete(index) {
    try {
      const response = await fetch(`/api/transactions/${index}`, {
        method: 'DELETE'
      });
      
      if (response.ok) {
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
    push(`/edit/${index}`);
  }
</script>

<div class="register-page">
  <h1>Transaction Register</h1>
  
  {#if errorMessage}
    <div class="error-message">{errorMessage}</div>
  {/if}
  
  {#if isLoading}
    <div class="loading">Loading transactions...</div>
  {:else if transactions.length === 0}
    <div class="empty-state">
      <p>No transactions available</p>
      <button class="add-transaction-btn" on:click={() => push('/add')}>Add Your First Transaction</button>
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
</div>

<style>
  .register-page {
    width: 100%;
  }
  
  h1 {
    margin-bottom: 20px;
  }
  
  .loading, .empty-state {
    margin: 40px 0;
    text-align: center;
    font-style: italic;
    color: #777;
  }
  
  .empty-state button {
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
  
  .error-message {
    background: #ffecec;
    color: #e74c3c;
    padding: 15px;
    border-radius: 4px;
    margin: 20px 0;
    border-left: 4px solid #e74c3c;
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