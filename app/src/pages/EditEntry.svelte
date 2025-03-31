<script>
  import { onMount } from 'svelte';
  import { push } from 'svelte-spa-router';
  import { format } from 'date-fns';
  
  export let params = {};
  
  let transaction = null;
  let date = '';
  let payee = '';
  let isCleared = true;
  let postings = [];
  let accounts = [];
  let isLoading = true;
  let isSubmitting = false;
  let errorMessage = '';
  
  onMount(async () => {
    try {
      // Load accounts for autocomplete
      const accountsResponse = await fetch('/api/accounts');
      if (accountsResponse.ok) {
        accounts = await accountsResponse.json();
      }
      
      // Load transaction to edit
      const transactionResponse = await fetch(`/api/transactions/${params.index}`);
      if (transactionResponse.ok) {
        transaction = await transactionResponse.json();
        date = transaction.date;
        payee = transaction.payee;
        isCleared = !transaction.pending;
        
        if (transaction.postings && transaction.postings.length > 0) {
          postings = transaction.postings.map(p => ({
            account: p.account,
            amount: p.amount,
            comment: p.comment || ''
          }));
        } else {
          // Fallback if no postings data
          postings = [
            { account: transaction.account, amount: transaction.amount, comment: '' },
            { account: '', amount: '', comment: '' }
          ];
        }
      } else {
        errorMessage = 'Failed to load transaction';
      }
    } catch (error) {
      console.error('Error loading data:', error);
      errorMessage = 'Network error occurred';
    } finally {
      isLoading = false;
    }
  });
  
  function addPosting() {
    postings = [...postings, { account: '', amount: '', comment: '' }];
  }
  
  function removePosting(index) {
    postings = postings.filter((_, i) => i !== index);
  }
  
  async function handleSubmit() {
    if (!date || !payee || postings.some(p => !p.account)) {
      errorMessage = 'Please fill out all required fields';
      return;
    }
    
    isSubmitting = true;
    errorMessage = '';
    
    try {
      const response = await fetch(`/api/transactions/${params.index}`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({
          date,
          payee,
          isCleared,
          postings
        })
      });
      
      if (response.ok) {
        push('/register');
      } else {
        const error = await response.json();
        errorMessage = error.message || 'Failed to update entry';
      }
    } catch (error) {
      console.error('Error updating entry:', error);
      errorMessage = 'Network error occurred';
    } finally {
      isSubmitting = false;
    }
  }
</script>

<div class="edit-entry">
  <h1>Edit Transaction</h1>
  
  {#if isLoading}
    <div class="loading">Loading transaction data...</div>
  {:else if errorMessage}
    <div class="error-message">{errorMessage}</div>
  {:else}
    <form on:submit|preventDefault={handleSubmit}>
      <div class="form-group">
        <label for="date">Date</label>
        <input 
          id="date" 
          type="text" 
          bind:value={date} 
          placeholder="YYYY/MM/DD"
          required
        />
      </div>
      
      <div class="form-group">
        <label for="payee">Payee/Description</label>
        <input 
          id="payee" 
          type="text" 
          bind:value={payee} 
          placeholder="Grocery Store, Rent Payment, etc."
          required
        />
      </div>
      
      <div class="form-group checkbox">
        <label>
          <input type="checkbox" bind:checked={isCleared} />
          Mark as cleared
        </label>
      </div>
      
      <h2>Postings</h2>
      <div class="postings">
        {#each postings as posting, i}
          <div class="posting-item">
            <div class="posting-header">
              <h3>Posting {i + 1}</h3>
              {#if postings.length > 2}
                <button 
                  type="button" 
                  class="remove-btn" 
                  on:click={() => removePosting(i)}
                >
                  Remove
                </button>
              {/if}
            </div>
            
            <div class="posting-fields">
              <div class="form-group">
                <label for="account-{i}">Account</label>
                <input 
                  id="account-{i}" 
                  type="text" 
                  bind:value={posting.account} 
                  placeholder="Assets:Checking"
                  required
                  list="accounts-list"
                />
              </div>
              
              <div class="form-group">
                <label for="amount-{i}">Amount</label>
                <input 
                  id="amount-{i}" 
                  type="text" 
                  bind:value={posting.amount} 
                  placeholder="$100.00"
                />
                <small>Leave last amount blank for automatic balancing</small>
              </div>
              
              <div class="form-group">
                <label for="comment-{i}">Comment (Optional)</label>
                <input 
                  id="comment-{i}" 
                  type="text" 
                  bind:value={posting.comment} 
                  placeholder="; groceries"
                />
              </div>
            </div>
          </div>
        {/each}
      </div>
      
      <div class="form-actions">
        <button type="button" class="add-posting-btn" on:click={addPosting}>
          Add Another Posting
        </button>
        
        <div class="submit-area">
          <button type="button" class="cancel-btn" on:click={() => push('/register')}>
            Cancel
          </button>
          <button type="submit" class="submit-btn" disabled={isSubmitting}>
            {isSubmitting ? 'Saving...' : 'Update Entry'}
          </button>
        </div>
      </div>
    </form>
  {/if}
  
  <datalist id="accounts-list">
    {#each accounts as account}
      <option value={account} />
    {/each}
  </datalist>
</div>

<style>
  .edit-entry {
    background: white;
    border-radius: 8px;
    padding: 20px;
    box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
    max-width: 800px;
    margin: 0 auto;
  }
  
  .loading {
    margin: 40px 0;
    text-align: center;
    font-style: italic;
    color: #777;
  }
  
  h1, h2 {
    margin-top: 0;
  }
  
  h2 {
    margin-top: 30px;
    padding-bottom: 10px;
    border-bottom: 1px solid #eee;
  }
  
  .form-group {
    margin-bottom: 20px;
  }
  
  label {
    display: block;
    margin-bottom: 8px;
    font-weight: bold;
  }
  
  input[type="text"] {
    width: 100%;
    padding: 8px;
    border: 1px solid #ddd;
    border-radius: 4px;
    font-size: 16px;
  }
  
  .checkbox {
    display: flex;
    align-items: center;
  }
  
  .checkbox label {
    display: flex;
    align-items: center;
    font-weight: normal;
  }
  
  .checkbox input {
    margin-right: 8px;
  }
  
  .posting-item {
    background: #f9f9f9;
    border-radius: 4px;
    padding: 15px;
    margin-bottom: 20px;
  }
  
  .posting-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 15px;
  }
  
  .posting-header h3 {
    margin: 0;
    font-size: 1rem;
  }
  
  .form-actions {
    margin-top: 30px;
    display: flex;
    flex-direction: column;
    gap: 20px;
  }
  
  .submit-area {
    display: flex;
    justify-content: space-between;
  }
  
  button {
    padding: 10px 20px;
    border: none;
    border-radius: 4px;
    cursor: pointer;
    font-weight: bold;
  }
  
  .add-posting-btn {
    background: #f0f0f0;
    color: #333;
    width: 100%;
  }
  
  .add-posting-btn:hover {
    background: #e0e0e0;
  }
  
  .remove-btn {
    background: #e74c3c;
    color: white;
    padding: 5px 10px;
    font-size: 0.8rem;
  }
  
  .cancel-btn {
    background: #95a5a6;
    color: white;
  }
  
  .submit-btn {
    background: #f39c12;
    color: white;
    min-width: 120px;
  }
  
  .submit-btn:hover:not(:disabled) {
    background: #e67e22;
  }
  
  .submit-btn:disabled {
    background: #95a5a6;
    cursor: not-allowed;
  }
  
  .error-message {
    background: #ffecec;
    color: #e74c3c;
    padding: 10px;
    border-radius: 4px;
    margin-bottom: 20px;
    border-left: 4px solid #e74c3c;
  }
  
  small {
    display: block;
    color: #7f8c8d;
    margin-top: 5px;
    font-size: 0.8rem;
  }
</style> 