<script>
  import { push, location } from 'svelte-spa-router';

  const menuItems = [
    { path: '/', label: 'Dashboard' },
    { path: '/balance', label: 'Balance' },
    { path: '/register', label: 'Register' },
    { path: '/budget', label: 'Budget' },
    { path: '/cleared', label: 'Cleared' }
  ];
</script>

<aside>
  <nav>
    <ul>
      {#each menuItems as item}
        <li class:active={$location === item.path}>
          <a href="#{item.path}" on:click|preventDefault={() => push(item.path)}>
            {item.label}
          </a>
        </li>
      {/each}
    </ul>
  </nav>
</aside>

<style>
  aside {
    width: 200px;
    background-color: #34495e;
    color: white;
  }

  nav {
    padding: 20px 0;
  }

  ul {
    list-style: none;
    padding: 0;
    margin: 0;
  }

  li {
    padding: 0;
  }

  a {
    display: block;
    padding: 12px 20px;
    color: #ecf0f1;
    text-decoration: none;
    transition: background-color 0.2s;
  }

  a:hover {
    background-color: #2c3e50;
  }

  .active a {
    background-color: #2c3e50;
    border-left: 4px solid #3498db;
    font-weight: bold;
  }
</style> 