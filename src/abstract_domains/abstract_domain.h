/**
 * Abstract domains.
 * 
 * Defines which abstractions are available.
 * 
 * @file abstract_domain.h
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
 */
#ifndef ABSTRACT_DOMAIN_H
#define ABSTRACT_DOMAIN_H

/**
 * Type of an abstract domain.
 */
typedef enum {
    ABSTRACT_DOMAIN_INTERVAL = 0x1,
    ABSTRACT_DOMAIN_RAF = 0x2,
    ABSTRACT_DOMAIN_HYBRID = 0x4
} AbstractDomainType;

#endif
