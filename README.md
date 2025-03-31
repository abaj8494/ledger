This repository is my full-stack wrapper around [ledger-cli](https://ledger-cli.org/).

In essence, I wanted to view, query and mutate my ledger file beyond just from within Emacs.

As such I have built a lightweight front-end in Svelte.js and used C++ in the backend to facilitate this functionality. The codebase was initialised by Cursor and Claude 3.7 Sonnet Max.

The [site](https://ledger.abaj.ai) is locked by nginx, but the source code is available here should you be interested in the project and wish to deploy it for your own use.

You can deploy the backend at port 3001 with
`cd app/cpp_backend && ./build.sh`

And the frontend can be deployed with `cd app && npm run build`.