<script>
  import { push } from 'svelte-spa-router';
</script>

<header>
  <div class="logo">
    <h1>Ledger Web</h1>
  </div>
  <div class="actions">
    <button class="add-entry" on:click={() => push('/add')}>Add Entry</button>
  </div>
</header>

<style>
  header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 0 20px;
    height: 60px;
    background-color: #2c3e50;
    color: white;
    box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
  }

  .logo h1 {
    margin: 0;
    font-size: 1.5rem;
  }

  .actions {
    display: flex;
    gap: 10px;
  }

  .add-entry {
    background-color: #27ae60;
    border: none;
    color: white;
    padding: 8px 16px;
    border-radius: 4px;
    cursor: pointer;
    font-weight: bold;
  }

  .add-entry:hover {
    background-color: #2ecc71;
  }
</style> 