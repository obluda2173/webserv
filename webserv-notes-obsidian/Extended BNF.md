1.  **Square Brackets** (`[]`)

    -   indicate optional elements

2.  **Repetition** (`*`):

    ``` {.bnf}
    <l>*<m>element
    ```

    -   at least `l` and at most `m` occurences of element

    -   default are `0` and `infinity`

        -   so it allows any number

            ``` {.bnf}
            *element
            ```

    -   at least 1

        ``` {.bnf}
        1*element
        ```

    -   at most 1

        ``` {.bnf}
        *1element
        ```

    -   1 or 2

        ``` {.bnf}
        1*2element
        ```

3.  **Specific Repition** (`<n>element`)

    -   equivalent to (`<n>*<n>element`)

4.  **Lists** (`<l>#<m>element`)

    -   seperated by commas
    -   at least `l` and at most `m` elements
    -   with this, something like `(element *("," element))` becomes `1#element`
        -   NULL ELEMENTS ARE ALLOWED BUT DO NOT CONTRIBUTE TO THE COUNT OF ELEMENTS `(element),,(element)`

5.  **Semicolons** (`;`) = Comments
