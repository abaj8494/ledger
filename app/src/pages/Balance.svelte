<script>
  import { onMount } from 'svelte';
  
  let balanceData = [];
  let isLoading = true;
  let errorMessage = '';
  
  onMount(async () => {
    try {
      const response = await fetch('/api/balance');
      
      if (response.ok) {
        balanceData = await response.json();
      } else {
        errorMessage = 'Failed to load balance data';
      }
    } catch (error) {
      console.error('Error fetching balance data:', error);
      errorMessage = 'Error connecting to server';
    } finally {
      isLoading = false;
    }
  });
</script>

<div class="balance-page">
  <h1>Balance Report</h1>
  
  {#if isLoading}
    <div class="loading">Loading balance data...</div>
  {:else if errorMessage}
    <div class="error-message">{errorMessage}</div>
  {:else if balanceData.length === 0}
    <div class="empty-state">No balance data available</div>
  {:else}
    <div class="balance-table-container">
      <table class="balance-table">
        <thead>
          <tr>
            <th class="amount-column">Amount</th>
            <th class="account-column">Account</th>
          </tr>
        </thead>
        <tbody>
          {#each balanceData as item}
            <tr class="account-row" style="padding-left: {item.level * 20}px">
              <td class="amount-cell">{item.amount}</td>
              <td class="account-cell" style="padding-left: {item.level * 20}px">
                {item.account}
              </td>
            </tr>
          {/each}
          <tr class="total-row">
            <td class="amount-cell total">{balanceData[balanceData.length - 1]?.total || ''}</td>
            <td class="account-cell"></td>
          </tr>
        </tbody>
      </table>
    </div>
  {/if}
</div>

<style>
  .balance-page {
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
  
  .error-message {
    background: #ffecec;
    color: #e74c3c;
    padding: 15px;
    border-radius: 4px;
    margin: 20px 0;
    border-left: 4px solid #e74c3c;
  }
  
  .balance-table-container {
    background: white;
    border-radius: 8px;
    box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
    overflow: hidden;
  }
  
  .balance-table {
    width: 100%;
    border-collapse: collapse;
  }
  
  .balance-table th {
    background: #f5f5f5;
    padding: 12px 15px;
    text-align: left;
    border-bottom: 2px solid #ddd;
    font-weight: bold;
  }
  
  .balance-table .amount-column {
    width: 200px;
    text-align: right;
  }
  
  .balance-table .account-column {
    text-align: left;
  }
  
  .balance-table td {
    padding: 10px 15px;
    border-bottom: 1px solid #eee;
  }
  
  .balance-table .amount-cell {
    font-family: monospace;
    text-align: right;
    white-space: nowrap;
  }
  
  .balance-table .account-cell {
    text-align: left;
  }
  
  .balance-table .total-row {
    font-weight: bold;
    border-top: 2px solid #ddd;
  }
  
  .balance-table .total {
    border-top: 1px solid #333;
    padding-top: 15px;
  }
</style> 