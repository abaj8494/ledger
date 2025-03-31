<script>
  import { onMount } from 'svelte';
  
  let clearedData = [];
  let isLoading = true;
  let errorMessage = '';
  
  onMount(async () => {
    try {
      const response = await fetch('/api/cleared');
      
      if (response.ok) {
        clearedData = await response.json();
      } else {
        errorMessage = 'Failed to load cleared data';
      }
    } catch (error) {
      console.error('Error fetching cleared data:', error);
      errorMessage = 'Error connecting to server';
    } finally {
      isLoading = false;
    }
  });
</script>

<div class="cleared-page">
  <h1>Cleared Report</h1>
  
  {#if isLoading}
    <div class="loading">Loading cleared data...</div>
  {:else if errorMessage}
    <div class="error-message">{errorMessage}</div>
  {:else if clearedData.length === 0}
    <div class="empty-state">No cleared data available</div>
  {:else}
    <div class="cleared-table-container">
      <table class="cleared-table">
        <thead>
          <tr>
            <th class="cleared-column">Cleared</th>
            <th class="pending-column">Pending</th>
            <th class="date-column">Last Cleared</th>
            <th class="account-column">Account</th>
          </tr>
        </thead>
        <tbody>
          {#each clearedData as item}
            <tr class="account-row">
              <td class="amount-cell">{item.cleared}</td>
              <td class="amount-cell">{item.pending}</td>
              <td class="date-cell">{item.lastCleared}</td>
              <td class="account-cell" style="padding-left: {item.level * 20}px">
                {item.account}
              </td>
            </tr>
          {/each}
          <tr class="total-row">
            <td class="amount-cell total">{clearedData[clearedData.length - 1]?.totalCleared || ''}</td>
            <td class="amount-cell total">{clearedData[clearedData.length - 1]?.totalPending || ''}</td>
            <td class="date-cell total">{clearedData[clearedData.length - 1]?.lastCleared || ''}</td>
            <td class="account-cell"></td>
          </tr>
        </tbody>
      </table>
    </div>
  {/if}
</div>

<style>
  .cleared-page {
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
  
  .cleared-table-container {
    background: white;
    border-radius: 8px;
    box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
    overflow: hidden;
  }
  
  .cleared-table {
    width: 100%;
    border-collapse: collapse;
  }
  
  .cleared-table th {
    background: #f5f5f5;
    padding: 12px 15px;
    text-align: left;
    border-bottom: 2px solid #ddd;
    font-weight: bold;
  }
  
  .cleared-table .cleared-column,
  .cleared-table .pending-column {
    width: 120px;
    text-align: right;
  }
  
  .cleared-table .date-column {
    width: 120px;
    text-align: center;
  }
  
  .cleared-table .account-column {
    text-align: left;
  }
  
  .cleared-table td {
    padding: 10px 15px;
    border-bottom: 1px solid #eee;
  }
  
  .cleared-table .amount-cell {
    font-family: monospace;
    text-align: right;
    white-space: nowrap;
  }
  
  .cleared-table .date-cell {
    text-align: center;
  }
  
  .cleared-table .account-cell {
    text-align: left;
  }
  
  .cleared-table .total-row {
    font-weight: bold;
    border-top: 2px solid #ddd;
  }
  
  .cleared-table .total {
    border-top: 1px solid #333;
    padding-top: 15px;
  }
</style>