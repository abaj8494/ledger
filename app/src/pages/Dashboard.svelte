<script>
  import { onMount } from 'svelte';
  import { push } from 'svelte-spa-router';
  
  let accountSummary = [];
  let recentTransactions = [];
  let isLoading = true;
  
  onMount(async () => {
    try {
      const [summaryRes, transactionsRes] = await Promise.all([
        fetch('/api/summary'),
        fetch('/api/transactions?limit=5')
      ]);
      
      if (summaryRes.ok && transactionsRes.ok) {
        accountSummary = await summaryRes.json();
        recentTransactions = await transactionsRes.json();
      }
    } catch (error) {
      console.error('Error fetching dashboard data:', error);
    } finally {
      isLoading = false;
    }
  });
</script>

<div class="dashboard">
  <h1>Dashboard</h1>

  {#if isLoading}
    <div class="loading">Loading dashboard data...</div>
  {:else}
    <div class="summary-cards">
      <div class="card">
        <h2>Account Summary</h2>
        <div class="account-list">
          {#if accountSummary.length === 0}
            <p>No account data available</p>
          {:else}
            {#each accountSummary as account}
              <div class="account-item">
                <span class="account-name">{account.name}</span>
                <span class="account-balance">{account.balance}</span>
              </div>
            {/each}
          {/if}
        </div>
        <button on:click={() => push('/balance')}>View All Accounts</button>
      </div>

      <div class="card">
        <h2>Recent Transactions</h2>
        <div class="transaction-list">
          {#if recentTransactions.length === 0}
            <p>No recent transactions</p>
          {:else}
            {#each recentTransactions as transaction}
              <div class="transaction-item">
                <div class="transaction-date">{transaction.date}</div>
                <div class="transaction-payee">{transaction.payee}</div>
                <div class="transaction-amount">{transaction.amount}</div>
              </div>
            {/each}
          {/if}
        </div>
        <button on:click={() => push('/register')}>View All Transactions</button>
      </div>
    </div>
  {/if}
</div>

<style>
  .dashboard {
    width: 100%;
  }

  h1 {
    margin-bottom: 20px;
  }

  .loading {
    margin: 40px 0;
    text-align: center;
    font-style: italic;
    color: #777;
  }

  .summary-cards {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
    gap: 20px;
  }

  .card {
    background: white;
    border-radius: 8px;
    padding: 20px;
    box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
  }

  h2 {
    margin-top: 0;
    margin-bottom: 15px;
    font-size: 1.2rem;
    border-bottom: 1px solid #eee;
    padding-bottom: 10px;
  }

  .account-list, .transaction-list {
    margin-bottom: 15px;
  }

  .account-item, .transaction-item {
    display: flex;
    justify-content: space-between;
    padding: 8px 0;
    border-bottom: 1px solid #f0f0f0;
  }

  .account-balance {
    font-family: monospace;
    font-weight: bold;
  }

  .transaction-date {
    min-width: 100px;
    color: #666;
  }

  .transaction-payee {
    flex: 1;
    padding: 0 10px;
  }

  .transaction-amount {
    font-family: monospace;
    text-align: right;
  }

  button {
    width: 100%;
    background: #3498db;
    color: white;
    border: none;
    padding: 8px;
    border-radius: 4px;
    cursor: pointer;
    margin-top: 10px;
  }

  button:hover {
    background: #2980b9;
  }
</style> 