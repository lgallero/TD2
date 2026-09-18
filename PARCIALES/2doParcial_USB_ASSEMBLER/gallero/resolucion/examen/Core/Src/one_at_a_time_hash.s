.syntax     unified
.thumb

.section    .text
.align      2

.global one_at_a_time_hash
.type one_at_a_time_hash, %function

/*
uint32_t one_at_a_time_hash(uint8_t* datos, uint32_t len)
{
	uint32_t i;
	uint32_t hash = 0;
	for(i=0; i<len; i++)
	{
		hash += datos[i];
		hash += hash << 10;
		hash ^= hash >> 6;
	}

	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;
	return hash;
}
*/

// r0 = datos
// r1 = len
// r2 = hash
// r4 = temporal

one_at_a_time_hash:
	push {r4, lr} 			// Guardo en pila r4

	cmp r1, #0    			// len == 0
	bne init_hash 			// si len no es 0, entro

	movs r0, #0	  			// Si len es 0, cargo r0 = 0 y lo devuelvo sin leer en memoria
	pop {r4, pc}


init_hash :
	movs r2, #0   			// hash = 0

loop:
//hash += datos[i];
	ldrb r4, [r0], #1 		// r4 = datos[i]; datos++
	adds r2, r2, r4  		// hash += datos[i]

//hash += hash << 10;
	mov r4, r2
	lsl r4, r4, #10   		// r4 = hash << 10
	adds r2, r2, r4	  		// hash = hash + (hash << 10)

//hash ^= hash >> 6;
	mov r4,r2
	lsr r4, r4, #6    		// r4 = hash >> 6
	eor r2 , r2, r4   		// hash = hash ^ (hash >> 6)

	subs r1,r1, #1    		// len = len-1
	bne loop          		// sigo hasta que len == 0

// termina el loop

/*
	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;
*/

//hash += hash << 3;
	mov r4, r2
	lsl r4, r4, #3 			// r4 = hash << 3
	adds r2, r2, r4 		// hash = hash + (hash << 3)

//hash ^= hash >> 11;
	mov r4,r2
	lsr r4, r4, #11   		 // r4 = hash >> 11
	eor r2 , r2, r4   		// hash = hash ^ (hash >> 11)

//hash += hash << 15
	mov r4, r2
	lsl r4, r4, #15   		// r4 = hash << 15
	adds r2, r2, r4	 		// hash = hash + (hash << 15)


	mov r0, r2 				// return hash;
	pop {r4, pc} 			// recupero r4 y vuelvo
