/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _Colors.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erian <erian@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/06 17:33:49 by erian             #+#    #+#             */
/*   Updated: 2025/04/09 19:38:03 by erian            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// ===== RAINBOW COLOR HEADER =====

# define ERROR			"\e[48;5;196m[ ERROR ]\e[0m \e[38;5;160m"
# define WARNING		"\e[48;5;214m[ WARNING ]\e[0m \e[38;5;208m"
# define SUCCESS		"\e[48;5;40m[ SUCCESS ]\e[0m \e[38;5;46m"
# define INFO			"\e[48;5;33m[ INFO ]\e[0m \e[38;5;39m"
# define DEBUG			"\e[48;5;93m[ DEBUG ]\e[0m \e[38;5;129m"

// === RED COLORS ===
# define RED			"\e[38;5;196m"
# define DARK_RED		"\e[38;5;88m"
# define RED_B			"\e[48;5;160m"

// === ORANGE COLORS ===
# define ORANGE			"\e[38;5;214m"
# define DARK_ORANGE	"\e[38;5;166m"
# define ORANGE_B		"\e[48;5;208m"

// === YELLOW COLORS ===
# define YELLOW			"\e[38;5;226m"
# define DARK_YELLOW	"\e[38;5;220m"
# define YELLOW_B		"\e[48;5;184m"

// === GREEN COLORS ===
# define GREEN			"\e[38;5;46m"
# define DARK_GREEN		"\e[38;5;22m"
# define GREEN_B		"\e[48;5;40m"

// === BLUE COLORS ===
# define BLUE			"\e[38;5;39m"
# define DARK_BLUE		"\e[38;5;21m"
# define SKY_BLUE		"\e[38;5;45m"
# define BLUE_B			"\e[48;5;33m"

// === INDIGO COLORS ===
# define INDIGO			"\e[38;5;57m"
# define DARK_INDIGO	"\e[38;5;54m"
# define INDIGO_B		"\e[48;5;63m"

// === VIOLET/MAGENTA COLORS ===
# define VIOLET			"\e[38;5;129m"
# define MAGENTA		"\e[38;5;201m"
# define DARK_MAGENTA	"\e[38;5;90m"
# define VIOLET_B		"\e[48;5;93m"

// === STYLES ===
# define BOLD			"\e[1m"
# define ITALIC			"\e[3m"
# define UNDERLINE		"\e[4m"
# define STRIKETHROUGH	"\e[9m"

// === RESET COLOR ===
# define NC				"\e[0m"

// === FANCY DEBUGGING TEXT ===
# define P_DEBUG		"\e[48;5;129m=====[ DEBUG MODE ]=====\e[0m \e[38;5;201m"
# define T_DEBUG		"\e[38;5;214m"
