# Contributing Guidelines

Contributing entails that all contributions follow the [MPL 2.0 license](https://github.com/buffet/kiwmi/blob/master/LICENSE).

## Opening Issues

- Ensure that bugs have proper steps to reproduce
- Include program version, along with any system specs that may help identify the issue

## Committing

Commit messages should be both _clear_ and _descriptive_. If possible, commit titles should start with a verb describing the change. Don't be shy to include additional information such as motivation for the change in the commit body. Be sure to ensure other developers will be able to understand _why_ a specific change has occurred in the future.

## Code formatting

To ensure consistent formatting, use `clang-format -i **/*.[ch]` (beware, not all shells support that glob). Don't worry too much about forgetting to do so, the pipeline will remind you by failing on your commit/PR. You can also make this your pre-commit git hook (see `man 5 githooks`) to avoid committing any misformatted code:

```sh
#!/bin/sh

# Check if there're any formatting errors
find . '(' -name .git -o -path ./build ')' -prune \
    -o -type f -name '*.[ch]' -print |
  xargs -d '\n' clang-format --dry-run >/dev/null 2>&1

if [ $? -ne 0 ]; then
  echo 'Code formatting is wrong, fix it before commit' >/dev/stderr
  exit 1
fi
```
