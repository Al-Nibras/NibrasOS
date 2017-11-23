;*****************************************************************
;*			@Author: Abdulaziz Alfaifi <al-nibras>               *
;*			@Date:   2017-08-11T17:27:54+03:00					 *
;*			@Email:  al-nibras@outlook.sa						 *
;*			@Project: NibrasOS									 *
;*			@Filename: crti.s								     *
;*			@Last modified by:   al-nibras						 *
;*			@Last modified time: 2017-11-23T22:57:48+03:00		 *
;*****************************************************************

SECTION init
global _init
_init:
   push ebp
   mov ebp, esp

   ; gcc will nicely put the contents of crtbegin.o's .init section here.

SECTION fini
global _fini
_fini:
   push ebp
   mov ebp, esp
   ; gcc will nicely put the contents of crtbegin.o's .fini section here.
