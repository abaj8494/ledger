<script>
  import { onMount } from 'svelte';
  
  let budgetData = [];
  let isLoading = true;
  let errorMessage = '';
  
  onMount(async () => {
    try {
      const response = await fetch('/api/budget');
      
      if (response.ok) {
        budgetData = await response.json();
      } else {
        errorMessage = 'Failed to load budget data';
      }
    } catch (error) {
      console.error('Error fetching budget data:', error);
      errorMessage = 'Error connecting to server';
    } finally {
      isLoading = false;
    }
  });
</script>

<div class="budget-page">
  <h1>Budget Report</h1>
  
  {#if isLoading}
    <div class="loading">Loading budget data...</div>
  {:else if errorMessage}
    <div class="error-message">{errorMessage}</div>
  {:else if budgetData.length === 0}
    <div class="empty-state">No budget data available</div>
  {:else}
    <div class="budget-table-container">
      <table class="budget-table">
        <thead>
          <tr>
            <th class="actual-column">Actual</th>
            <th class="budget-column">Budget</th>
            <th class="remaining-column">Remaining</th>
            <th class="percent-column">%</th>
            <th class="account-column">Account</th>
          </tr>
        </thead>
        <tbody>
          {#each budgetData as item}
            <tr class="account-row">
              <td class="amount-cell">{item.actual}</td>
              <td class="amount-cell">{item.budget}</td>
              <td class="amount-cell">{item.remaining}</td>
              <td class="percent-cell">{item.percent}</td>
              <td class="account-cell" style="padding-left: {item.level * 20}px">
                {item.account}
              </td>
            </tr>
          {/each}
          <tr class="total-row">
            <td class="amount-cell total">{budgetData[budgetData.length - 1]?.totalActual || ''}</td>
            <td class="amount-cell total">{budgetData[budgetData.length - 1]?.totalBudget || ''}</td>
            <td class="amount-cell total">{budgetData[budgetData.length - 1]?.totalRemaining || ''}</td>
            <td class="percent-cell total">{budgetData[budgetData.length - 1]?.totalPercent || ''}</td>
            <td class="account-cell"></td>
          </tr>
        </tbody>
      </table>
    </div>
  {/if}
</div>

<style>
  .budget-page {
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
  
  .budget-table-container {
    background: white;
    border-radius: 8px;
    box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
    overflow: hidden;
  }
  
  .budget-table {
    width: 100%;
    border-collapse: collapse;
  }
  
  .budget-table th {
    background: #f5f5f5;
    padding: 12px 15px;
    text-align: left;
    border-bottom: 2px solid #ddd;
    font-weight: bold;
  }
  
  .budget-table .actual-column,
  .budget-table .budget-column,
  .budget-table .remaining-column {
    width: 120px;
    text-align: right;
  }
  
  .budget-table .percent-column {
    width: 60px;
    text-align: right;
  }
  
  .budget-table .account-column {
    text-align: left;
  }
  
  .budget-table td {
    padding: 10px 15px;
    border-bottom: 1px solid #eee;
  }
  
  .budget-table .amount-cell {
    font-family: monospace;
    text-align: right;
    white-space: nowrap;
  }
  
  .budget-table .percent-cell {
    text-align: right;
  }
  
  .budget-table .account-cell {
    text-align: left;
  }
  
  .budget-table .total-row {
    font-weight: bold;
    border-top: 2px solid #ddd;
  }
  
  .budget-table .total {
    border-top: 1px solid #333;
    padding-top: 15px;
  }
</style> 