What it is
==========

-   BNF is used to describe the grammar of a language in a precise and unambiguous way

-   **Purpose**:

    -   BNF is used to describe the grammar of a language in a precise and unambiguous way, which is especially useful for parser development and compiler construction.

-   **Structure**:

    -   **Non-Terminals**: Symbols that can be further expanded. They denote abstract syntactic categories and are usually represented with angle brackets (e.g., `<expression>`).
    -   **Terminals**: Basic symbols that cannot be broken down further. They represent actual characters or tokens in the language (e.g., keywords, operators, literals).

-   **Productions**:

    -   A BNF grammar consists of a set of production rules, each defining how a non-terminal can be replaced with a combination of non-terminals and terminals.
    -   The general form of a production rule is `<non-terminal> ::` \<sequence of terminals and/or non-terminals\>=.

-   **Examples**:

    -   For a simple arithmetic expression grammar:

            <expression> ::= <term> | <expression> "+" <term> | <expression> "-" <term>
            <term> ::= <factor> | <term> "*" <factor> | <term> "/" <factor>
            <factor> ::= <number> | "(" <expression> ")"
            <number> ::= <digit> | <number> <digit>
            <digit> ::= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"

[[Standard BNF Terminals]]
=================================================================

[[Extended BNF]]
=================================================================
